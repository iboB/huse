# Dev Notes

## TODO

* Trim sajson - remove `string` and replace with `std::string_view`, remove `literal`
* add dev mode tests which test assertions
* stronger exception types: add int code, add stack as vector
* json::StreamDeseriazlier with no out of order reads
* how to create a keyStream to write? Perhaps use small_vector?
	* Instead of holding a pimpl to struct { stream, buf } hold optionals for stream and buf separately, buf can be either json redirect or itlib::mem_ostreambuf for small_vector. Then set small_vector data as key
