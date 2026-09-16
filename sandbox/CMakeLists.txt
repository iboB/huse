# Copyright (c) Borislav Stanimirov
# SPDX-License-Identifier: MIT
#
add_executable(huse-sandbox sandbox.cpp)
target_link_libraries(huse-sandbox huse::huse)
set_target_properties(huse-sandbox PROPERTIES FOLDER sandbox)
