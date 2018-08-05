/*

authored from 2015-2018 by Micha Mettke and Andreas Koerner

LICENSE:
    This software is dual-licensed to the public domain and under the following
    license: you are granted a perpetual, irrevocable license to copy, modify,
    publish and distribute this file as you see fit.

*/

struct nk_context;
typedef int (*nkapp_callback)(struct nk_context* ctx, int w, int h, void* userdata);
int nkapp_run(nkapp_callback cb, void* userdata, int winwidth, int winheight, const char* wintitle);
