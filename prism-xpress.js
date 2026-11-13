// Extends Prism grammars with Xpress Optimization API tokens.
// Sources: vim/c.vim, vim/cpp.vim, vim/java.vim, vim/cs.vim, vim/vb.vim,
//          vim/bas.vim, vim/py.vim, vim/R.vim, vim/m.vim
//          (FICO Xpress Optimization 9.9, perlwebapp vim definitions)
// All xpress tokens → #cc0000 (HiLink xpressFunction Operator in each vim file).
// (C) Copyright 2026 Fair Isaac Corporation

(function (Prism) {

    var patterns = {
        // C: XPRS/XPRM/XPRB/XPRD API, NLS solver (XNLS*), SLP (XSLP*),
        //    binary driver (BINDRV* uppercase, bindrv* lowercase)
        'c': /\bXPRS[a-zA-Z0-9_]*|\bXPRM[a-zA-Z0-9_]*|\bXPRB[a-zA-Z0-9_]*|\bXPRD[a-zA-Z0-9_]*|\bXNLS[a-zA-Z0-9_]*|\bXSLP[a-zA-Z0-9_]*|\bBINDRV[a-zA-Z0-9_]*|\bbindrv[a-zA-Z0-9_]*/,

        // C++ (cpp.vim lists only XPRS/XPRM/XPRB)
        'cpp': /\bXPRS[a-zA-Z0-9_]*|\bXPRM[a-zA-Z0-9_]*|\bXPRB[a-zA-Z0-9_]*/,

        // Java: XPRS/XPRM/XPRB/XPRD class/method prefixes, BinDrv wrapper
        'java': /\bXPRS[a-zA-Z0-9_]*|\bXPRM[a-zA-Z0-9_]*|\bXPRB[a-zA-Z0-9_]*|\bXPRD[a-zA-Z0-9_]*|\bBinDrv[a-zA-Z0-9_]+/,

        // C#: XPRS/XPRM/XPRB + SLP (cs.vim)
        'csharp': /\bXPRS[a-zA-Z0-9_]*|\bXPRM[a-zA-Z0-9_]*|\bXPRB[a-zA-Z0-9_]*|\bXSLP[a-zA-Z0-9_]*/,

        // VB.NET (.vb) and VBA/Basic (.bas) share this grammar in Prism
        'visual-basic': /\bXPRS[a-zA-Z0-9_]*|\bXPRM[a-zA-Z0-9_]*|\bXPRB[a-zA-Z0-9_]*|(?:prob|mosel|model|proc|Optimizer|mySet|varr|vindex|sets|symb)\.[a-zA-Z0-9_]+/,

        // Python: top-level xpress module, branchobj and problem classes (py.vim)
        'python': /\b(?:xpress|branchobj|problem)\b/,

        // R: named R-API functions + xprs_* lowercase prefix + xpress:::UPPER namespace
        'r': /\b(?:readprob|createprob|setprobname|getdblattrib|getintattrib|setintcontrol|setdblcontrol|mipoptimize|chgobjsense|addrows|setindicators|addsets|setlogfile)\b|xprs_[a-z0-9_]+|xpress:::[A-Z0-9_]+/,

        // MATLAB: xprs* function prefix (m.vim syn match) and moselexec keyword
        'matlab': /xprs[a-zA-Z_]+|\bmoselexec\b/
    };

    // Rebuild the grammar object with 'xpress' prepended so it takes priority
    // over class-name / built-in matching in the base grammar.
    function prependToken(lang, pattern) {
        var original = Prism.languages[lang];
        if (!original) { return; }
        var rebuilt = { xpress: pattern };
        for (var key in original) {
            if (Object.prototype.hasOwnProperty.call(original, key)) {
                rebuilt[key] = original[key];
            }
        }
        Prism.languages[lang] = rebuilt;
    }

    for (var lang in patterns) {
        if (Object.prototype.hasOwnProperty.call(patterns, lang)) {
            prependToken(lang, patterns[lang]);
        }
    }

}(Prism));
