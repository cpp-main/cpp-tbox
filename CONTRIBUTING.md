# Contributing to cpp-tbox

## Coding style

Follow most of Google's code style.  
refer to: https://google.github.io/styleguide/cppguide.html

Exceptions:

- Source file postfix: .cpp, not .cc
- Indentation: 4 spaces
- Name of class member function: `aaaBbb()`, example: `void setBindPort(int port);`
- Name of class static function: `AaaBbb()`, example: `static Object* CreateObject();`
- Name of class static variable: `_xxx_`, example: `static int _ref_count_;`
- Name of static function: `_AaaBbb()`, example: `static void _PrintIt(int v);`
- Name of static variable: `_xxx`, example: `static int _count;`
- Avoid using smart pointers unless you have to.

## Fix bugs

Fix it, then add corresponding unit test case in `xxx_test.cpp`.

## Add new components

If you add new components named `xxx`.  

1. You should added `xxx.cpp` and `xxx.h` files.  
2. You should add the corresponding `xxx_test.cpp` files in the same directory and implement its unit test cases, and make sure all unit tests pass.  
3. You'd better also implement the corresponding sample program to show how to use this module.  

File require:

- Formart: unix
- Encoding: utf-8

## Pull request

- Pull request to develop branch, not master.
