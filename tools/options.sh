#
# options.sh
#
# this little tool stores the database of possible command line options 
# and generates various output from it.
#
# I just don't want to maintain the list of options in that many places
#
# We use a shell script for this and not the preprocessor because
# the output can be much better debugged this way.
#
# USAGE
#
#    options.sh output > destfile
#
# Possible Output Options:
#
#	   mdoc     for including inside an mandoc (BSD Man System) document
#	   parse    function parse_output() for inclusion in C source
#	   decl     C declarations of all OPTVARS with OPTTYPE
#	   init     initialisation of all OPTVARS with OPTINIT
#
# Internal Targets which should not be used from the outside:
#   optstr
#   actiondecl

# ----------------------------------------------------------------------------
# Globals / Utilities
# ----------------------------------------------------------------------------

OUTPUT=$1

die() { echo "error: $1" > /def/stderr; exit 1; }

# ----------------------------------------------------------------------------
# Header
# ----------------------------------------------------------------------------

if [ "$OUTPUT" == "parse" ]
then
OPTSTR=$($SHELL $0 optstr) # we call ourself to generate the getopt string
$SHELL $0 actiondecl
cat <<eof

#define RAISE_ERROR(err, msg, arg) do { \\
	return opts_set_err(opts, (err), (msg), (arg)); \\
} while(0)

static int
opts_set_err(struct opts* opts, int err, const char* msg, const char* arg)
{
	opts->errstr = msg;
	opts->errarg = arg;
	return err;
}

int
opts_parse(struct opts* opts, void* user, int argc, char** argv)
{
	char ch;
	int err; 
	const char* errstr;
	
	while ((ch = getopt(argc, argv, "$OPTSTR")) != -1) {
		switch (ch) {
eof
elif [ "$OUTPUT" == "decl" ]
then
cat <<eof
struct opts {
	/* error reporting string */
	const char* errstr;
	
	/* error argument which failed */
	const char* errarg;

eof
elif [ "$OUTPUT" == "init" ]
then
cat <<eof
void opts_init(struct opts* opts) {
	opts->errstr = 0;
	opts->errarg = "<unknown>";
eof
elif [ "$OUTPUT" == "help" ]
then
cat <<eof
void opts_help() {
eof
fi

# ----------------------------------------------------------------------------
# Called for every Options
# ----------------------------------------------------------------------------

makeopt() {
	if [ "$OUTPUT" == "parse" ]
	then
		echo "			case '$OPTCHAR': {"
		
		if [ -n "$OPTACTION" ]
		then
			echo "				if((err = $OPTACTION(user, optarg, &errstr)) < 0)"
			echo "					RAISE_ERROR(err, errstr, optarg);"	
		else
			echo "$OPTPARSE" | sed -e 's/^/			/' # indent the body
		fi
		echo "			} break;"

	elif [ "$OUTPUT" == "optstr" ]
	then
		if [ -z "$OPTARGNAME" ]
		then
			echo -n $OPTCHAR
		else
			echo -n "$OPTCHAR:"
		fi

	elif [ "$OUTPUT" == "mdoc" ]
	then
		if [ -z "$OPTARGNAME" ]
		then
			echo ".It Fl $OPTCHAR"
		else
			echo ".It Fl $OPTCHAR Ar $OPTARGNAME"
		fi

		if [ -z "$OPTDEFAULT" ]
		then
			echo "$OPTDESC"
		else
			echo "$OPTDESC [Default: '$OPTDEFAULT']"
		fi

	elif [ "$OUTPUT" == "decl" ]
	then
		if [ -n "$OPTTYPE" ] && [ -n "$OPTVAR" ]
		then
			echo ""
			echo "	/* $OPTDESC */" | sed -E 's|^([^	][^/][^*])|	   \1|';
			echo "	$OPTTYPE $OPTVAR;"
		fi

	elif [ "$OUTPUT" == "init" ]
	then
		if [ -n "$OPTTYPE" ] && [ -n "$OPTVAR" ] && [ -n "$OPTINIT" ]
		then
			echo "	opts->$OPTVAR = $OPTINIT;";
		fi

	elif [ "$OUTPUT" == "actiondecl" ]
	then
		if [ -n "$OPTACTION" ]
		then
			echo "int $OPTACTION(void*, const char*, const char**);"
		fi

	elif [ "$OUTPUT" == "help" ]
	then
		echo "-$OPTCHAR        $OPTDESC" | 
			sed -E 's|^([^-])|          \1|' |
			sed -E 's|^|  |' |
			sed -E 's|^|	puts("|' |
			sed -E 's|$|");|'
	fi
}

# ----------------------------------------------------------------------------
# The actual Option List
# ----------------------------------------------------------------------------

. ./options.conf

# ----------------------------------------------------------------------------
# Footer
# ----------------------------------------------------------------------------


if [ "$OUTPUT" == "parse" ]
then
cat <<eof
			default:
				RAISE_ERROR(OPTS_ERR_PARSE, "no valid option", argv[optind-1]);
		};
	}
	
	return optind;
}
eof
elif [ "$OUTPUT" == "decl" ]
then
cat <<eof
};

enum {
	  OPTS_OK = 0
	, OPTS_ERR       = -1
	, OPTS_ERR_PARSE = -2
	, OPTS_ERR_ALLOC = -3
};

/* initialise the option structure. */
void opts_init(struct opts* opts);

/* parse argc and argv into struct ops.
	Return Values: 
		>=0 : okay
		 <0 : Failure. Compare OPTS_ERR..., and use opts->erridx and 
		      opts->errstr for error reporting */
int opts_parse(struct opts* opts, void* user, int argc, char** argv);

void opts_help();
eof
elif [ "$OUTPUT" == "init" ]
then
cat <<eof
}
eof
elif [ "$OUTPUT" == "help" ]
then
cat <<eof
}
eof
fi
