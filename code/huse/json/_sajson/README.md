# sajson

Embedding the library sajson by Chad Austin from https://github.com/chadaustin/sajson/

Copyright &copy; 2012-2021 Chad Austin

## Local modifications

* Fixed integer parsing to cover the full 32-bit signed range
* Renamed namespace to huse::json::sajson so avoid ODR clashes with other users of sajson with potentially different versions
* Made sajson::value copyable
* Disable &lt;string&gt; include
* Fixed unused arg warnings
* Disable MSVC warning for non-standard extension with empty array
