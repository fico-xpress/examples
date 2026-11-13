// Prism.js language definition for Mosel (.mos)
// Derived from vim/mos.vim (FICO Xpress Optimization 9.9)
// Colour mapping matches the %COLOURS hash in example.pl
// (C) Copyright 2026 Fair Isaac Corporation

Prism.languages.mosel = {
    // Annotations: (!@ ... !) block or !@ to end of line (grey) — must be before comment
    'annotation': {
        pattern: /\(!@[\s\S]*?!\)|!@[^\r\n]*/,
        greedy: true
    },
    // Block comments: (! ... !) — listed after annotation so (!@ is already consumed
    'comment-block': {
        pattern: /\(![\s\S]*?!\)/,
        greedy: true,
        alias: 'comment'
    },
    // Line comments: ! to end of line, not !@ (annotation) and not (! (block comment)
    'comment': {
        pattern: /(?<!\()![^@\r\n][^\r\n]*/,
        greedy: true
    },
    // Strings: "..." or '...' or `...` (purple)
    'string': {
        pattern: /["'`](?:[^"'`\\]|\\.)*["'`]/,
        greedy: true
    },
    // Constants: CT_*, EP_*, F_*, INFINITY, MAX_*, M_E, M_PI, NAN, STRUCT_*, true/false (blue, same as Statement)
    'constant': /\b(?:true|false|TRUE|FALSE|CT_BIN|CT_CONT|CT_EQ|CT_FREE|CT_GEQ|CT_INT|CT_LEQ|CT_PINT|CT_RNG|CT_SEC|CT_SINT|CT_SOS1|CT_SOS2|CT_UNB|EP_HEX|EP_MAX|EP_MIN|EP_MPS|EP_STRIP|F_APPEND|F_BINARY|F_DELCLOSE|F_ERROR|F_INPUT|F_IOERR|F_LINBUF|F_OUTPUT|F_SILENT|F_TEXT|INFINITY|IOERR_IN|IOERR_OUT|MAX_INT|MAX_REAL|M_E|M_PI|NAN|STRUCT_ARRAY|STRUCT_CONST|STRUCT_LIST|STRUCT_NATTYPE|STRUCT_NIL|STRUCT_PROBLEM|STRUCT_RECORD|STRUCT_REF|STRUCT_ROUTINE|STRUCT_SET|STRUCT_UNION|STRUCT_USRTYPE)\b/,
    // Keywords / statements (blue)
    'keyword': {
        pattern: /\bend-(?:declarations|do|function|if|model|procedure|record|requirements|uses)?\b|\b(?:array|as|boolean|break|case|constant|counter|declarations|do|dynamic|elif|else|end|evaluation|forall|forward|from|function|hashmap|if|imports|include|initialisations|initializations|integer|linctr|list|model|mpvar|namespace|next|nssearch|nsgroup|of|options|package|parameters|procedure|public|range|real|record|repeat|requirements|return|set|shared|string|then|to|until|uses|version|while|with|any|mpproblem|returned|ARRAY|AS|BOOLEAN|BREAK|CASE|CONSTANT|COUNTER|DECLARATIONS|DO|DYNAMIC|ELIF|ELSE|EVALUATION|FORALL|FORWARD|FROM|FUNCTION|HASHMAP|IF|IMPORTS|INCLUDE|INITIALISATIONS|INITIALIZATIONS|INTEGER|LINCTR|LIST|MODEL|MPVAR|NAMESPACE|NEXT|NSSEARCH|NSGROUP|OF|OPTIONS|PACKAGE|PARAMETERS|PROCEDURE|PUBLIC|RANGE|REAL|RECORD|REPEAT|REQUIREMENTS|RETURN|SET|SHARED|STRING|THEN|TO|UNTIL|USES|VERSION|WHILE|WITH)\b/
    },
    // Operators: and/or/not/in/mod/div/sum/forall etc. (blue, same as Statement in vim)
    'operator-word': /\b(?:and|count|div|in|inter|max|min|mod|not|or|prod|sum|union|is|is_binary|is_continuous|is_free|is_integer|is_partint|is_semcont|is_semint|is_sos1|is_sos2|AND|COUNT|DIV|IN|INTER|MAX|MIN|MOD|NOT|OR|PROD|SUM|UNION|IS|IS_BINARY|IS_CONTINUOUS|IS_FREE|IS_INTEGER|IS_PARTINT|IS_SEMCONT|IS_SEMINT|IS_SOS1|IS_SOS2)\b/,
    // DSO / package functions (special teal: #1D8F96)
    'dso': /\b(?:command|clearmipdir|clearmodcut|delbasis|getiis|getlb|getprobstat|getub|initglobal|fixmipentities|fixglobal|getrange|loadbasis|loadprob|maximize|minimize|maximise|minimise|readbasis|readdirs|savebasis|savestate|setcallback|setucbdata|setlb|setmipdir|setmodcut|setub|writebasis|writedirs|writeprob|getname|isintegral|addcuts|setinitval|indicator|implies|pwlin|nlctr|pws|basis|boolvar|logctr|mpsol|qexp|robustctr|uncertain|uncertainctr|cpctr|cpvar|cpfloatvar|cptask|cpresource|cp_minimize|cp_maximize|cp_schedule|cp_show_sol|cp_find_next_sol|setdomain|insightminimize|insightminimise|insightmaximize|insightmaximise|insightgetmode|insightpopulate|insightdispatch|fdelete|fmove|fcopy|getcwd|getenv|getfsize|getfstat|getsysstat|gettime|makedir|removedir|formattext|textfmt|system|qsort|text|date|time|datetime|SQLconnect|SQLdisconnect|SQLexecute|SQLreadinteger|SQLreadreal|SQLreadstring|SQLupdate|SQLgetiparam|SQLgetrparam|SQLgetsparam|SQLparam|wait|waitfor|dropnextevent|getnextevent|getid|Model|Event|Mosel|setdefstream|connect|disconnect|compile|load|unload|run|send|getfromid|getclass|getvalue|setmodpar|setworkdir|xmldoc|getnode|getnodes|addnode|setattr|svgaddgroup|svgaddarrow|svgaddcircle|svgaddellipse|svgaddfile|svgaddimage|svgaddline|svgaddpie|svgaddpoint|svgaddpolygon|svgaddrectangle|svgaddtext|svgaddxmltext|svgclosing|svgcolor|svgdelobj|svgerase|svggetlastobj|svgpause|svgrefresh|svgsave|svgsetgraphlabels|svgsetgraphscale|svgsetstyle|svgwaitclose|svggetgraphviewbox|svgsetgraphviewbox|disc|diskdata)\b/,
    // Built-in functions (blue #0086b3)
    'function': /\b(?:abs|arctan|asproc|assert|bitflip|bitneg|bitset|bitshift|bittest|bitval|ceil|clsarrsrtndx|compare|cos|create|currentdate|currenttime|cutelt|cutfirst|cuthead|cutlast|cuttail|datablock|delcell|dumpcallstack|exists|exit|exp|exportprob|fclose|fflush|finalise|finalize|findfirst|findlast|floor|fopen|fselect|fskipline|fwrite|fwrite_|fwriteln|fwriteln_|getact|getchar|getcoeff|getcoeffs|getdual|getelt|geteltype|getfid|getfirst|getfname|gethead|getlast|getnbdim|getobjval|getparam|getrcost|getreadcnt|getreverse|getsize|getslack|getsol|getstruct|gettail|gettype|gettypeid|getvars|isdefined|isdynamic|iseof|isfinite|ishidden|isinf|isnan|isodd|ln|localsetparam|log|makesos1|makesos2|maxlist|memoryuse|minlist|newmuid|publish|random|read|readln|reset|restoreparam|reverse|round|setcoeff|sethidden|setioerr|setmatherr|setname|setparam|setrandseed|setrange|settype|sin|splithead|splittail|sqrt|strfmt|substr|timestamp|unpublish|versionnum|versionstr|write|write_|writeln|writeln_|_)\b/,
    // Symbol operators (red #cc0000) — ! excluded (handled by comment patterns)
    'operator': /[:+\-*\/=<>^]+|[()[\]{},;]/,
    // Numbers
    'number': /\b\d+(?:\.\d+)?(?:[eE][+-]?\d+)?\b/
};
