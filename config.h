/* See LICENSE file for copyright and license details. */

#include <X11/XF86keysym.h>
/* appearance */
static const unsigned int borderpx  = 2;        /* border pixel of windows */
static const unsigned int snap      = 32;       /* snap pixel */
static const int swallowfloating    = 1;        /* 1 means swallow floating windows by default */
static const unsigned int systraypinning = 0;   /* 0: sloppy systray follows selected monitor, >0: pin systray to monitor X */
static const unsigned int systrayonleft = 0;    /* 0: systray in the right corner, >0: systray on left of status text */
static const unsigned int systrayspacing = 2;   /* systray spacing */
static const int systraypinningfailfirst = 1;   /* 1: if pinning fails, display systray on the first monitor, False: display systray on the last monitor*/
static const int showsystray        = 1;        /* 0 means no systray */
static const unsigned int gappih    = 40;       /* horiz inner gap between windows */
static const unsigned int gappiv    = 40;       /* vert inner gap between windows */
static const unsigned int gappoh    = 40;       /* horiz outer gap between windows and screen edge */
static const unsigned int gappov    = 40;       /* vert outer gap between windows and screen edge */
static       int smartgaps          = 0;        /* 1 means no outer gap when there is only one window */
static const int showbar            = 1;        /* 0 means no bar */
static const int topbar             = 1;        /* 0 means bottom bar */
static const char *fonts[]          = { "monospace:size=10" };
static const char dmenufont[]       = "monospace:size=10";
static const char col_gray1[]       = "#222222";
static const char col_gray2[]       = "#444444";
static const char col_gray3[]       = "#bbbbbb";
static const char col_gray4[]       = "#eeeeee";
static const char col_cyan[]        = "#005577";
/* static const char *colors[][3]      = { */
/* 	/1*               fg         bg         border   *1/ */
/* 	[SchemeNorm] = { col_gray3, col_gray1, col_gray2 }, */
/* 	[SchemeSel]  = { col_gray4, col_cyan,  col_cyan  }, */
/* }; */


static char normbgcolor[]           = "#222222";
static char normbordercolor[]       = "#444444";
static char normfgcolor[]           = "#bbbbbb";
static char selfgcolor[]            = "#eeeeee";
static char selbordercolor[]        = "#770000";
static char selbgcolor[]            = "#005577";
static const char *colors[][3] = {
       /*               fg           bg           border   */
       [SchemeNorm] = { normfgcolor, normbgcolor, normbordercolor },
       [SchemeSel]  = { selfgcolor,  selbgcolor,  selbordercolor  },
};

/* tagging */
static const char *tags[] = { "1", "2", "3", "4", "5", "6", "7", "8", "9" };

static const Rule rules[] = {
	/* xprop(1):
	 *	WM_CLASS(STRING) = instance, class
	 *	WM_NAME(STRING) = title
	 */
	/* class          , instance , title , tags mask , isfloating ,  isterminal  noswallow  monitor */
	{ "Gimp"          , NULL     , NULL  , 0         , 1          ,  0,           0,        -1 }       ,
	{ "slack"         , NULL     , NULL  , 1 << 7    , 0          ,  0,          -1,        -1 }       ,
	{ "Slack"         , NULL     , NULL  , 1 << 7    , 0          ,  0,           0,        -1 }       ,
	{ "kwalletd5"     , NULL     , NULL  , 1 << 7    , 0          ,  0,           1,        -1 }       ,
	{ "discord"       , NULL     , NULL  , 1 << 8    , 0          ,  0,           0,        -1 }       ,
	{ "Prospect Mail" , NULL     , NULL  , 1 << 7    , 0          ,  0,           0,        -1 }       ,
	{ "St"            , NULL     , NULL  , 0         , 0          ,  1,           0,        -1 }       ,
	{ "qutebrowser"   , NULL     , NULL  , 1 << 2    , 0          ,  0,           0,        -1 }       ,
};

/* layout(s) */
static const float mfact     = 0.55; /* factor of master area size [0.05..0.95] */
static const int nmaster     = 1;    /* number of clients in master area */
static const int resizehints = 1;    /* 1 means respect size hints in tiled resizals */
static const int lockfullscreen = 1; /* 1 will force focus on the fullscreen window */

#define FORCE_VSPLIT 1  /* nrowgrid layout: force two clients to always split vertically */
#include "vanitygaps.c"

static const Layout layouts[] = {
	/* symbol     arrange function */
	{ "[]=",      tile },    /* first entry is default */
	{ "><>",      NULL },    /* no layout function means floating behavior */
	{ "[M]",      monocle },
	{ "[@]",      spiral },
	{ "[\\]",     dwindle },
	{ "H[]",      deck },
	{ "TTT",      bstack },
	{ "===",      bstackhoriz },
	{ "HHH",      grid },
	{ "###",      nrowgrid },
	{ "---",      horizgrid },
	{ ":::",      gaplessgrid },
	{ "|M|",      centeredmaster },
	{ ">M>",      centeredfloatingmaster },
	{ "><>",      NULL },    /* no layout function means floating behavior */
	{ NULL,       NULL },
};

void
cyclelayout(const Arg *arg) {
    Layout *l;
    for (l = (Layout *)layouts; l != selmon->lt[selmon->sellt]; l++);
    if (arg->i > 0) {
        if (l == &layouts[LENGTH(layouts) - 1])
            setlayout(&((Arg) { .v = &layouts[0] }));
        else
            setlayout(&((Arg) { .v = l + 1 }));
    } else {
        if (l == layouts)
            setlayout(&((Arg) { .v = &layouts[LENGTH(layouts) - 1] }));
        else
            setlayout(&((Arg) { .v = l - 1 }));
    }
}

/* key definitions */
#define MODKEY Mod4Mask
#define TAGKEYS(KEY,TAG)                                                                                               \
       &((Keychord){1, {{MODKEY, KEY}},                                        view,           {.ui = 1 << TAG} }), \
       &((Keychord){1, {{MODKEY|ControlMask, KEY}},                            toggleview,     {.ui = 1 << TAG} }), \
       &((Keychord){1, {{MODKEY|ShiftMask, KEY}},                              tag,            {.ui = 1 << TAG} }), \
       &((Keychord){1, {{MODKEY|ControlMask|ShiftMask, KEY}},                  toggletag,      {.ui = 1 << TAG} }),

/* helper for spawning shell commands in the pre dwm-5.0 fashion */
#define SHCMD(cmd) { .v = (const char*[]){ "/bin/sh", "-c", cmd, NULL } }

/* commands */
static char dmenumon[2] = "0"; /* component of dmenucmd, manipulated in spawn() */
static const char *dmenucmd[] = { "dmenu_run", "-m", dmenumon, "-fn", dmenufont, "-nb", col_gray1, "-nf", col_gray3, "-sb", col_cyan, "-sf", col_gray4, NULL };
static const char *termcmd[]  = { "st", "-e", "zsh" };
static const char *OpenJira[]  = { "/home/sudqi/dotfiles/scripts/Open.sh", NULL};
static const char *OpenNvim[]  = { "st", "-e", "nvim"};
// Shouldn't be needed. I'm using autorandr now
static const char *SetDisplay[]  = { "/home/sudqi/dotfiles/scripts/display_set.sh", NULL};
static const char *ExtendLeft[]  = { "/home/sudqi/dotfiles/scripts/display_set.sh" , "-l"};
static const char *ExtendRight[]  = { "/home/sudqi/dotfiles/scripts/display_set.sh", "-r"};
static const char *goToWindow[]  = { "/home/sudqi/dotfiles/scripts/goto.sh", NULL};
static const char *goToZoom[]  = { "/home/sudqi/dotfiles/scripts/goToZoom.sh", NULL};
static const char *increaseVolume[]  = { "/home/sudqi/dotfiles/scripts/increaseVolume.sh", NULL};
static const char *decreaseVolume[]  = { "/home/sudqi/dotfiles/scripts/decreaseVolume.sh", NULL};
static const char *MuteVolume[]  = { "/home/sudqi/dotfiles/scripts/MuteVolume.sh", NULL};
static const char *increaseBrightness[]  = { "/home/sudqi/dotfiles/scripts/changeScreenBrightness.sh", "10%+"};
static const char *decreaseBrightness[]  = { "/home/sudqi/dotfiles/scripts/changeScreenBrightness.sh", "10%-"};
static const char *Sleep[]  = { "systemctl", "suspend", NULL};
static const char *lock[]  = { "slock"};
static const char *ConnectToVpn[]  = { "/home/sudqi/dotfiles/scripts/connect_vpn.sh" , "sjawabreh"};

static Keychord *keychords[] = {
	/* modifier                     key        function        argument */
	&((Keychord){1,  {{MODKEY,                       XK_p}},       spawn,          {.v = dmenucmd } }),
	&((Keychord){1,  {{MODKEY|ShiftMask,             XK_Return}},  spawn,          {.v = termcmd } }),
    &((Keychord){2, {{MODKEY, XK_e},             {MODKEY, XK_e}},  spawn,          {.v = OpenNvim } }),
    &((Keychord){2, {{MODKEY, XK_e},             {MODKEY, XK_p}},  spawn,          {.v = SetDisplay } }),
    &((Keychord){2, {{MODKEY, XK_e},             {MODKEY, XK_r}},  spawn,          {.v = ExtendRight} }),
    &((Keychord){2, {{MODKEY, XK_e},             {MODKEY, XK_h}},  spawn,          {.v = ExtendLeft } }),
    &((Keychord){2, {{MODKEY, XK_e},             {MODKEY, XK_v}},  spawn,          {.v = ConnectToVpn } }),
    &((Keychord){2, {{MODKEY, XK_e},             {MODKEY, XK_s}},  spawn,          {.v = Sleep } }),
    &((Keychord){2, {{MODKEY, XK_e},             {MODKEY, XK_l}},  spawn,          {.v = lock } }),
	&((Keychord){1,  {{MODKEY,                       XK_b}},      togglebar,      {0} }),
    // not used for now because I'm using focusdir
	/* { MODKEY,                       XK_j,      focusstack,     {.i = +1 } }, */
	/* { MODKEY,                       XK_k,      focusstack,     {.i = -1 } }, */
	&((Keychord){1,  {{MODKEY,                       XK_h}},      focusdir,       {.i = 0 } }), // left
	&((Keychord){1,  {{MODKEY,                       XK_l}},      focusdir,       {.i = 1 } }), // right
	&((Keychord){1,  {{MODKEY,                       XK_k}},      focusdir,       {.i = 2 } }), // up
	&((Keychord){1,  {{MODKEY,                       XK_j}},      focusdir,       {.i = 3 } }), // down
	&((Keychord){1,  {{MODKEY,                       XK_i}},      incnmaster,     {.i = +1 } }),
	&((Keychord){1,  {{MODKEY,                       XK_d}},      incnmaster,     {.i = -1 } }),
	&((Keychord){1,  {{MODKEY|ShiftMask,             XK_h}},      setmfact,       {.f = -0.05} }),
	&((Keychord){1,  {{MODKEY|ShiftMask,             XK_l}},      setmfact,       {.f = +0.05} }),
	&((Keychord){1,  {{MODKEY|ShiftMask,             XK_j}},      setcfact,       {.f = +0.25} }),
	&((Keychord){1,  {{MODKEY|ShiftMask,             XK_k}},      setcfact,       {.f = -0.25} }),
	&((Keychord){1,  {{MODKEY|ShiftMask,             XK_o}},      setcfact,       {.f =  0.00} }),
	&((Keychord){1,  {{MODKEY,                       XK_Return}}, zoom,           {0} }),
	&((Keychord){1,  {{MODKEY|Mod1Mask,              XK_u}},      incrgaps,       {.i = +1 } }),
	&((Keychord){1,  {{MODKEY|Mod1Mask|ShiftMask,    XK_u}},      incrgaps,       {.i = -1 } }),
	&((Keychord){1,  {{MODKEY|Mod1Mask,              XK_i}},      incrigaps,      {.i = +1 } }),
	&((Keychord){1,  {{MODKEY|Mod1Mask|ShiftMask,    XK_i}},      incrigaps,      {.i = -1 } }),
	&((Keychord){1,  {{MODKEY|Mod1Mask,              XK_o}},      incrogaps,      {.i = +1 } }),
	&((Keychord){1,  {{MODKEY|Mod1Mask|ShiftMask,    XK_o}},      incrogaps,      {.i = -1 } }),
	&((Keychord){1,  {{MODKEY|Mod1Mask,              XK_6}},      incrihgaps,     {.i = +1 } }),
	&((Keychord){1,  {{MODKEY|Mod1Mask|ShiftMask,    XK_6}},      incrihgaps,     {.i = -1 } }),
	&((Keychord){1,  {{MODKEY|Mod1Mask,              XK_7}},      incrivgaps,     {.i = +1 } }),
	&((Keychord){1,  {{MODKEY|Mod1Mask|ShiftMask,    XK_7}},      incrivgaps,     {.i = -1 } }),
	&((Keychord){1,  {{MODKEY|Mod1Mask,              XK_8}},      incrohgaps,     {.i = +1 } }),
	&((Keychord){1,  {{MODKEY|Mod1Mask|ShiftMask,    XK_8}},      incrohgaps,     {.i = -1 } }),
	&((Keychord){1,  {{MODKEY|Mod1Mask,              XK_9}},      incrovgaps,     {.i = +1 } }),
	&((Keychord){1,  {{MODKEY|Mod1Mask|ShiftMask,    XK_9}},      incrovgaps,     {.i = -1 } }),
	&((Keychord){1,  {{MODKEY|Mod1Mask,              XK_0}},      togglegaps,     {0} }),
	&((Keychord){1,  {{MODKEY|Mod1Mask|ShiftMask,    XK_0}},      defaultgaps,    {0} }),
	&((Keychord){1,  {{MODKEY,                       XK_Tab}},    view,           {0} }),
	&((Keychord){1,  {{MODKEY|ShiftMask,             XK_c}},      killclient,     {0} }),
	&((Keychord){1,  {{MODKEY,                       XK_t}},      setlayout,      {.v = &layouts[0]} }),
	&((Keychord){1,  {{MODKEY,                       XK_f}},      setlayout,      {.v = &layouts[1]} }),
	&((Keychord){1,  {{MODKEY,                       XK_m}},      setlayout,      {.v = &layouts[2]} }),
	&((Keychord){1,  {{MODKEY|ShiftMask,             XK_m}},      setlayout,      {.v = &layouts[12]} }),
	/* { Mod1Mask,                     XK_space,  setlayout,      {0} }, */
	/* { Mod1Mask|ShiftMask,           XK_space,  togglefloating, {0} }, */
	&((Keychord){1,  {{MODKEY,                       XK_0}},      view,           {.ui = ~0 } }),
    &((Keychord){1,  {{MODKEY, XK_n}},                 cyclelayout,               {.i = +1} }), // cycle to next layout
	&((Keychord){1,  {{MODKEY|ShiftMask,             XK_0}},      tag,            {.ui = ~0 } }),
	&((Keychord){1,  {{MODKEY,                       XK_comma}},  focusmon,       {.i = -1 } }),
	&((Keychord){1,  {{MODKEY,                       XK_period}}, focusmon,       {.i = +1 } }),
	&((Keychord){1,  {{MODKEY|ShiftMask,             XK_comma}},  tagmon,         {.i = -1 } }),
	&((Keychord){1,  {{MODKEY|ShiftMask,             XK_period}}, tagmon,         {.i = +1 } }),
    &((Keychord){1,  {{MODKEY|ShiftMask,             XK_p}},     spawn,          {.v = goToWindow} }),
    /* &((Keychord){1,  {{MODKEY,                       XK_g}},      spawn,          {.v = goToWindow} }), */
    &((Keychord){1,  {{MODKEY,                       XK_w}},     spawn,          {.v = OpenJira} }),
    &((Keychord){2,  {{MODKEY,XK_o},                {0, XK_n}},   spawn,          {.v = OpenNvim} }),
    /* Keybindings for dunst notifications */
    &((Keychord){1,  {{ControlMask|ShiftMask,        XK_u}},      spawn,          SHCMD("dunstctl history-pop") }),
    &((Keychord){1,  {{ControlMask|ShiftMask,        XK_space}},  spawn,          SHCMD("dunstctl close") }),
    &((Keychord){1,  {{ControlMask|ShiftMask,        XK_d}},      spawn,          SHCMD("dunstctl close-all") }),
    &((Keychord){1,  {{ControlMask|ShiftMask,        XK_Return}}, spawn,          SHCMD("dunstctl action && dunstctl close") }),
	&((Keychord){1,  {{MODKEY|ShiftMask,             XK_s}},      spawn,          SHCMD( "maim -s | xclip -selection clipboard -t image/png && notify-send \"Screenshot Copied\" ")}),
	&((Keychord){1,  {{MODKEY,                       XK_z}},      spawn,          {.v = goToZoom} }),
	&((Keychord){1,  {{MODKEY,                       XK_Up}},     spawn,          {.v = increaseVolume} }),
	&((Keychord){1,  {{MODKEY,                       XK_Down}},   spawn,        {.v = decreaseVolume} }),
	&((Keychord){1,  {{MODKEY,                       XK_Right}},   spawn,          {.v = increaseBrightness} }),
	&((Keychord){1,  {{MODKEY,                       XK_Left}},   spawn,        {.v = decreaseBrightness} }),
    &((Keychord){1,  {{0, XF86XK_AudioRaiseVolume}}, spawn, {.v = increaseVolume} }),
    &((Keychord){1,  {{0, XF86XK_AudioLowerVolume}}, spawn, {.v = decreaseVolume} }),
    &((Keychord){1,  {{0, XF86XK_AudioMute}},        spawn, {.v = MuteVolume} }),
    //{ MODKEY, XK_v, spawn, SHCMD("sh -c 'mpv --no-border  --ontop \"$(dmenu -p Enter\\ YouTube\\ URL: )\"'") }),
    &((Keychord){1,  {{MODKEY, XK_v}},               spawn, SHCMD("clipcat-menu") }),
    //&((Keychord){1,  {{MODKEY, XK_v}}, spawn, SHCMD("prime-run mpv --no-border --geometry=100%x100% --ontop $(xclip -o -selection clipboard)") }),
	&((Keychord){1,  {{MODKEY|ShiftMask,             XK_q}},      quit,           {0} }),
	TAGKEYS(                        XK_1,                      0)
	TAGKEYS(                        XK_2,                      1)
	TAGKEYS(                        XK_3,                      2)
	TAGKEYS(                        XK_4,                      3)
	TAGKEYS(                        XK_5,                      4)
	TAGKEYS(                        XK_6,                      5)
	TAGKEYS(                        XK_7,                      6)
	TAGKEYS(                        XK_8,                      7)
	TAGKEYS(                        XK_9,                      8)
};

/* button definitions */
/* click can be ClkTagBar, ClkLtSymbol, ClkStatusText, ClkWinTitle, ClkClientWin, or ClkRootWin */
static const Button buttons[] = {
	/* click                event mask      button          function        argument */
	{ ClkLtSymbol,          0,              Button1,        setlayout,      {0} },
	{ ClkLtSymbol,          0,              Button3,        setlayout,      {.v = &layouts[2]} },
	{ ClkWinTitle,          0,              Button2,        zoom,           {0} },
	{ ClkStatusText,        0,              Button2,        spawn,          {.v = termcmd } },
	{ ClkClientWin,         MODKEY,         Button1,        movemouse,      {0} },
	{ ClkClientWin,         MODKEY,         Button2,        togglefloating, {0} },
	{ ClkClientWin,         MODKEY,         Button3,        resizemouse,    {0} },
	{ ClkTagBar,            0,              Button1,        view,           {0} },
	{ ClkTagBar,            0,              Button3,        toggleview,     {0} },
	{ ClkTagBar,            MODKEY,         Button1,        tag,            {0} },
	{ ClkTagBar,            MODKEY,         Button3,        toggletag,      {0} },
};

