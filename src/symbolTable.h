#ifndef _SYMBOLTABLE_H_
#define _SYMBOLTABLE_H_
#include <map>
#include <vector>
#include <string>
#include <stdio.h>
#include <stdlib.h>

// // // // // // // // // // // // // // // // // // // // 
//
// Some sample void * printing routines.   User should supply their own.
//
void pointerPrintNothing(void *dataASN_4);
void pointerPrintAddr(void *data);
void pointerPrintLongInteger(void *data);
void pointerPrintStr(void *data);

// // // // // // // // // // // // // // // // // // // // 
//
// Introduction
//
// This symbol table library supplies basic insert and lookup for
// symbols linked to void * pointers of data. The is expected to use
// ONLY the SymbolTable class and NOT the Scope class. The Scope class
// is used by SymbolTable in its implementation.
//
// Plenty of room for improvement inlcuding: better debugging setup,
// passing of refs rather than values and purpose built char *
// routines, and C support.
//
// WARNING: lookup will return NULL pointer if key is not in table.
// This means the void * cannot have zero as a legal value! Attempting
// to save a NULL pointer will get a error.
//
// A main() is commented out and has testing code in it.
//
// Robert Heckendorn   Apr 3, 2021
//


// // // // // // // // // // // // // // // // // // // // 
//
// Class: Scope
//
// Helper class for SymbolTable
//

class Scope {
private:
    static bool debugFlg;                      // turn on tedious debugging
    std::string name;                          // name of scope
    std::map<std::string , void *> symbols;    // use an ordered map (not as fast as unordered)

public:
    Scope(std::string newname);
    ~Scope();
    std::string scopeName();                   // returns name of scope
    void debug(bool state);                    // sets the debug flag to state
    void print(void (*printData)(void *));     // prints the table using the supplied function to print the void *
    void applyToAll(void (*action)(std::string , void *));  // applies func to all symbol/data pairs 
    bool insert(std::string sym, void *ptr);   // inserts a new ptr associated with symbol sym 
                                               // returns false if already defined
    void *lookup(std::string sym);             // returns the ptr associated with sym
                                               // returns NULL if symbol not found
};



// // // // // // // // // // // // // // // // // // // // 
//
// Class: SymbolTable
//
// Is a stack of scopes.   The global scope is created when the table is
// is constructed and remains for the lifetime of the object instance.
// SymbolTable manages nested scopes as a result.
// 

class SymbolTable {
private:
    std::vector<Scope *> stack;
    bool debugFlg;

public:
    SymbolTable();
    void debug(bool state);                          // sets the debug flags
    int depth();                                     // what is the depth of the scope stack?
    void print(void (*printData)(void *));           // print all scopes using data printing function
    void enter(std::string name);                    // enter a scope with given name
    void leave();                                    // leave a scope (not allowed to leave global)
    void *lookup(std::string sym);                   // returns ptr associated with sym anywhere in symbol table
                                                     // returns NULL if symbol not found
    void *lookupGlobal(std::string sym);             // returns ptr associated with sym in globals
                                                     // returns NULL if symbol not found
    bool insert(std::string sym, void *ptr);         // inserts new ptr associated with symbol sym in current scope
                                                     // returns false if already defined
    bool insertGlobal(std::string sym, void *ptr);   // inserts a new ptr associated with symbol sym 
                                                     // returns false if already defined
    void applyToAll(void (*action)(std::string , void *));        // apply func to all symbol/data pairs in local scope
    void applyToAllGlobal(void (*action)(std::string , void *));  // apply func to all symbol/data pairs in global scope
};

#endif
