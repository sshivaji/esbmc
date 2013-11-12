#include <cmdline.h>

#include <fstream>

#include "parseoptions.h"

const struct opt_templ esbmc_options[] = {
{ 0,	"no-inlining",		switc,		""	},
{ 0,	"program-only",		switc,		""	},
{ 0,	"program-too",		switc,		""	},
{ 0,	"function",		string,		""	},
{ 0,	"preprocess",		switc,		""	},
{ 0,	"slice-by-trace",	string,		""	},
{ 0,	"no-simplify",		switc,		""	},
{ 0,	"unwind",		number,		""	},
{ 0,	"unwindset",		string,		""	},
{ 0,	"slice-formula",	switc,		""	},
{ 0,	"debug-level",		number,		""	},
{ 0,	"no-bounds-check",	switc,		""	},
{ 0,	"z3-bv",		switc,		""	},
{ 0,	"z3-ir",		switc,		""	},
{ 0,	"z3",			switc,		""	},
{ 0,	"smt",			switc,		""	},
{ 0,	"outfile",		string,		""	},
{ 0,	"llvm-metadata",		string,		""	},
{ 0,	"no-pointer-check",	switc,		""	},
{ 0,	"document-subgoals",	switc,		""	},
{ 0,	"all-claims",		switc,		""	},
{ 'D',	"",			string,		""	},
{ 'I',	"",			string,		""	},
{ 0,	"depth",		number,		""	},
{ 0,	"no-div-by-zero-check",	switc,		""	},
{ 0,	"no-unwinding-assertions", switc,	""	},
{ 0,	"no-assume-guarantee",	switc,		""	},
{ 0,	"partial-loops",	switc,		""	},
{ 0,	"int-encoding",		switc,		""	},
{ 0,	"show-features",	switc,		""	},
{ 0,	"memory-leak-check",	switc,		""	},
{ 0,	"overflow-check",	switc,		""	},
{ 0,	"beautify-greedy",	switc,		""	},
{ 0,	"floatbv",		switc,		""	},
{ 0,	"fixedbv",		switc,		""	},
{ 0,	"no-assertions",	switc,		""	},
{ 0,	"gui",			switc,		""	},
{ 0,	"nan-check",		switc,		""	},
{ 0,	"16",			switc,		""	},
{ 0,	"32",			switc,		""	},
{ 0,	"64",			switc,		""	},
{ 0,	"little-endian",	switc,		""	},
{ 0,	"big-endian",		switc,		""	},
{ 0,	"show-goto-functions",	switc,		""	},
{ 0,	"show-counter-example",	switc,		""	},
{ 0,	"show-value-sets",	switc,		""	},
{ 0,	"xml-ui",		switc,		""	},
{ 0,	"show-loops",		switc,		""	},
{ 0,	"show-symbol-table",	switc,		""	},
{ 0,	"show-vcc",		switc,		""	},
{ 0,	"show-claims",		switc,		""	},
{ 0,	"claim",		number,		""	},
{ 0,	"atomicity-check",	switc,		""	},
{ 0,	"error-label",		string,		""	},
{ 0,	"verbosity",		number,		""	},
{ 0,	"binary",		string,		""	},
{ 0,	"no-library",		switc,		""	},
{ 0,	"version",		switc,		""	},
{ 0,	"i386-linux",		switc,		""	},
{ 0,	"i386-macos",		switc,		""	},
{ 0,	"i386-win32",		switc,		""	},
{ 0,	"ppc-macos",		switc,		""	},
{ 0,	"unsigned-char",	switc,		""	},
{ 0,	"arrays-uf-always",	switc,		""	},
{ 0,	"arrays-uf-never",	switc,		""	},
{ 0,	"no-lock-check",	switc,		""	},
{ 0,	"deadlock-check",	switc,		""	},
{ 0,	"string-abstraction",	switc,		""	},
{ 0,	"no-arch",		switc,		""	},
{ 0,	"eager",		switc,		""	},
{ 0,	"lazy",			switc,		""	},
{ 0,	"no-slice",		switc,		""	},
{ 0,	"uw-model",		switc,		""	},
{ 0,	"control-flow-test",	switc,		""	},
{ 0,	"round-to-nearest",	switc,		""	},
{ 0,	"round-to-plus-inf",	switc,		""	},
{ 0,	"round-to-minus-inf",	switc,		""	},
{ 0,	"round-to-zero",	switc,		""	},
{ 0,	"qf_aufbv",		switc,		""	},
{ 0,	"qf_auflira",		switc,		""	},
{ 0,	"btor",			switc,		""	},
{ 0,	"context-switch",	number,		"-1"	},
{ 0,	"time-slice",   	number,		"1"	},
{ 0,	"k-step",   	number,		"50"	},
{ 0,	"no-por",		switc,		""	},
{ 0,	"data-races-check",	switc,		""	},
{ 0,	"DFS",			switc,		""	},
{ 0,	"schedule",		switc,		""	},
{ 0,	"all-runs",		switc,		""	},
{ 0,	"timeout",		string,		""	},
{ 0,	"memlimit",		string,		""	},
{ 0,	"state-hashing",	switc,		""	},
{ 0,	"symex-trace",		switc,		""	},
{ 0,  "extended-try-analysis",    switc,    ""  },
{ 0,	"core-size",		number,		""	},
{ 0,	"smtlib-ileave-num",	number,		""	},
{ 0,	"direct-interleavings",	switc,		""	},
{ 0,	"show-ileave-points",	switc,		""	},
{ 0,	"checkpoint-file",	string,		""	},
{ 0,	"from-checkpoint",	switc,		""	},
{ 0,    "checkpoint-on-cex",    switc,          ""      },
{ 0,	"print-stack-traces",	switc,		""	},
{ 0,	"interactive-ileaves",	switc,		""	},
{ 0,	"round-robin",	switc,		""	},
{ 0,	"base-case",	switc,		""	},
{ 0,	"forward-condition",	switc,		""	},
{ 0,	"inductive-step",	switc,		""	},
{ 0,	"k-induction", switc, "" },
{ 0,	"break-at",		string,		""	},
{ 0,	"decide",		switc,		""	},
{ 0,	"enable-core-dump",     switc,		""	},
{ 0,	"memstats",		switc,		""	},
{ 0,	"dump-z3-assigns",	switc,		""	},
{ 0,	"smt-during-symex",	switc,		""	},
{ 0,	"smt-thread-guard",	switc,		""	},
{ 0,	"smt-symex-guard",	switc,		""	},
{ 0,	"ltl",			switc,		""	},
{ 0,	"symex-ssa-trace",	switc,		""	},
{ 0,	"double-assign-check",	switc,		""	},
{ 0,	"git-hash",		switc,		""	},
{ 0,	"no-align-check",	switc,		""	},
{ 0,	"quiet",	switc,		""	},
{ '?',	"",			switc,		""	},
{ 'h',	"",			switc,		""	},
{ 'I',	"",			string,		""	},
{ 0,	"help",			switc,		""	},
{ 0,	"",			switc,		""	}
};
