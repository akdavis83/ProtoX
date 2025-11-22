New command line interface
--------------------------

A new `qtc` command line tool has been added to make features more
discoverable and convenient to use. The `qtc` tool just calls other
executables and does not implement any functionality on its own.  Specifically
`qtc node` is a synonym for `qtcd`, `qtc gui` is a synonym for
`qtc-qt`, and `qtc rpc` is a synonym for `qtc-cli -named`. Other
commands and options can be listed with `qtc help`. The new tool does not
replace other tools, so all existing commands should continue working and there
are no plans to deprecate them.
