// Prism.js language definition for Mosel preprocessor files (.mosm)
// Derived from vim/mosm.vim and the "mos"/"mosm" branch of example.pl
// Requires prism-mosel.js (loaded first); adds the mmpp macro syntax on top of Mosel.
// (C) Copyright 2026 Fair Isaac Corporation

Prism.languages.mosm = Prism.languages.extend('mosel', {});

Prism.languages.insertBefore('mosm', 'annotation', {
    // Preprocessor lines: # at start of line (after optional blanks) — must precede all other tokens
    'comment-hash': {
        pattern: /(^[ \t]*)#.*/m,
        lookbehind: true,
        greedy: true,
        alias: 'comment'
    }
});

Prism.languages.insertBefore('mosm', 'constant', {
    // Macro references: $name (mmppMacro → Statement, same blue as keywords)
    'macro': {
        pattern: /\$[a-zA-Z_][a-zA-Z0-9_]*/,
        alias: 'keyword'
    }
});
