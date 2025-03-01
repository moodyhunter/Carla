/*
  ==============================================================================

   This file is part of the Water library.
   Copyright (c) 2016 ROLI Ltd.
   Copyright (C) 2017-2018 Filipe Coelho <falktx@falktx.com>

   Permission is granted to use this software under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license/

   Permission to use, copy, modify, and/or distribute this software for any
   purpose with or without fee is hereby granted, provided that the above
   copyright notice and this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND ISC DISCLAIMS ALL WARRANTIES WITH REGARD
   TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
   FITNESS. IN NO EVENT SHALL ISC BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT,
   OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
   USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
   TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
   OF THIS SOFTWARE.

  ==============================================================================
*/

#ifndef WATER_STRING_H_INCLUDED
#define WATER_STRING_H_INCLUDED

#include "CharPointer_UTF8.h"
#include "../memory/Memory.h"

#include <limits>
#include <string>

namespace water {

//==============================================================================
/**
    The Water String class!

    Using a reference-counted internal representation, these strings are fast
    and efficient, and there are methods to do just about any operation you'll ever
    dream of.

    @see StringArray, StringPairArray
*/
class String
{
public:
    //==============================================================================
    /** Creates an empty string.
        @see empty
    */
    String() noexcept;

    /** Creates a copy of another string. */
    String (const String& other) noexcept;

    /** Creates a string from a zero-terminated ascii text string.

        The string passed-in must not contain any characters with a value above 127, because
        these can't be converted to unicode without knowing the original encoding that was
        used to create the string. If you attempt to pass-in values above 127, you'll get an
        assertion.

        To create strings with extended characters from UTF-8, you should explicitly call
        String (CharPointer_UTF8 ("my utf8 string..")). It's *highly* recommended that you
        use UTF-8 with escape characters in your source code to represent extended characters,
        because there's no other way to represent unicode strings in a way that isn't dependent
        on the compiler, source code editor and platform.
    */
    String (const char* text);

    /** Creates a string from a string of 8-bit ascii characters.

        The string passed-in must not contain any characters with a value above 127, because
        these can't be converted to unicode without knowing the original encoding that was
        used to create the string. If you attempt to pass-in values above 127, you'll get an
        assertion.

        To create strings with extended characters from UTF-8, you should explicitly call
        String (CharPointer_UTF8 ("my utf8 string..")). It's *highly* recommended that you
        use UTF-8 with escape characters in your source code to represent extended characters,
        because there's no other way to represent unicode strings in a way that isn't dependent
        on the compiler, source code editor and platform.

        This will use up to the first maxChars characters of the string (or less if the string
        is actually shorter).
    */
    String (const char* text, size_t maxChars);

    //==============================================================================
    /** Creates a string from a UTF-8 character string */
    String (const CharPointer_UTF8 text);

    /** Creates a string from a UTF-8 character string */
    String (const CharPointer_UTF8 text, size_t maxChars);

    /** Creates a string from a UTF-8 character string */
    String (const CharPointer_UTF8 start, const CharPointer_UTF8 end);

    //==============================================================================
    /** Creates a string from a UTF-8 encoded std::string. */
    String (const std::string&);

    /** Creates a string from a StringRef */
    String (StringRef);

    //==============================================================================
    /** Creates a string from a single character. */
    static String charToString (water_uchar character);

    /** Destructor. */
    ~String() noexcept;

    /** This is the character encoding type used internally to store the string. */
    typedef CharPointer_UTF8 CharPointerType;

    //==============================================================================
    /** Generates a probably-unique 32-bit hashcode from this string. */
    int hashCode() const noexcept;

    /** Generates a probably-unique 64-bit hashcode from this string. */
    int64 hashCode64() const noexcept;

    /** Generates a probably-unique hashcode from this string. */
    size_t hash() const noexcept;

    /** Returns the number of characters in the string. */
    int length() const noexcept;

    //==============================================================================
    // Assignment and concatenation operators..

    /** Replaces this string's contents with another string. */
    String& operator= (const String& other) noexcept;

    /** Appends another string at the end of this one. */
    String& operator+= (const String& stringToAppend);
    /** Appends another string at the end of this one. */
    String& operator+= (const char* textToAppend);
    /** Appends another string at the end of this one. */
    String& operator+= (StringRef textToAppend);
    /** Appends a decimal number at the end of this string. */
    String& operator+= (int numberToAppend);
    /** Appends a decimal number at the end of this string. */
    String& operator+= (long numberToAppend);
    /** Appends a decimal number at the end of this string. */
    String& operator+= (int64 numberToAppend);
    /** Appends a decimal number at the end of this string. */
    String& operator+= (uint64 numberToAppend);
    /** Appends a character at the end of this string. */
    String& operator+= (char characterToAppend);
    /** Appends a character at the end of this string. */
    String& operator+= (water_uchar characterToAppend);

    /** Appends a string to the end of this one.

        @param textToAppend     the string to add
        @param maxCharsToTake   the maximum number of characters to take from the string passed in
    */
    void append (const String& textToAppend, size_t maxCharsToTake);

    /** Appends a string to the end of this one.

        @param startOfTextToAppend  the start of the string to add. This must not be a nullptr
        @param endOfTextToAppend    the end of the string to add. This must not be a nullptr
    */
    void appendCharPointer (const CharPointerType startOfTextToAppend,
                            const CharPointerType endOfTextToAppend);

    /** Appends a string to the end of this one.

        @param startOfTextToAppend  the start of the string to add. This must not be a nullptr
        @param endOfTextToAppend    the end of the string to add. This must not be a nullptr
    */
    template <class CharPointer>
    void appendCharPointer (const CharPointer startOfTextToAppend,
                            const CharPointer endOfTextToAppend)
    {
        wassert (startOfTextToAppend.getAddress() != nullptr && endOfTextToAppend.getAddress() != nullptr);

        size_t extraBytesNeeded = 0, numChars = 1;

        for (CharPointer t (startOfTextToAppend); t != endOfTextToAppend && ! t.isEmpty(); ++numChars)
            extraBytesNeeded += CharPointerType::getBytesRequiredFor (t.getAndAdvance());

        if (extraBytesNeeded > 0)
        {
            const size_t byteOffsetOfNull = getByteOffsetOfEnd();

            preallocateBytes (byteOffsetOfNull + extraBytesNeeded);
            CharPointerType (addBytesToPointer (text.getAddress(), (int) byteOffsetOfNull))
                .writeWithCharLimit (startOfTextToAppend, (int) numChars);
        }
    }

    /** Appends a string to the end of this one. */
    void appendCharPointer (const CharPointerType textToAppend);

    /** Appends a string to the end of this one.

        @param textToAppend     the string to add
        @param maxCharsToTake   the maximum number of characters to take from the string passed in
    */
    template <class CharPointer>
    void appendCharPointer (const CharPointer textToAppend, size_t maxCharsToTake)
    {
        if (textToAppend.getAddress() != nullptr)
        {
            size_t extraBytesNeeded = 0, numChars = 1;

            for (CharPointer t (textToAppend); numChars <= maxCharsToTake && ! t.isEmpty(); ++numChars)
                extraBytesNeeded += CharPointerType::getBytesRequiredFor (t.getAndAdvance());

            if (extraBytesNeeded > 0)
            {
                const size_t byteOffsetOfNull = getByteOffsetOfEnd();

                preallocateBytes (byteOffsetOfNull + extraBytesNeeded);
                CharPointerType (addBytesToPointer (text.getAddress(), (int) byteOffsetOfNull))
                    .writeWithCharLimit (textToAppend, (int) numChars);
            }
        }
    }

    /** Appends a string to the end of this one. */
    template <class CharPointer>
    void appendCharPointer (const CharPointer textToAppend)
    {
        appendCharPointer (textToAppend, std::numeric_limits<size_t>::max());
    }

    //==============================================================================
    // Comparison methods..

    /** Returns true if the string contains no characters.
        Note that there's also an isNotEmpty() method to help write readable code.
        @see containsNonWhitespaceChars()
    */
    inline bool isEmpty() const noexcept                    { return text.isEmpty(); }

    /** Returns true if the string contains at least one character.
        Note that there's also an isEmpty() method to help write readable code.
        @see containsNonWhitespaceChars()
    */
    inline bool isNotEmpty() const noexcept                 { return ! text.isEmpty(); }

    /** Resets this string to be empty. */
    void clear() noexcept;

    /** Case-insensitive comparison with another string. */
    bool equalsIgnoreCase (const String& other) const noexcept;

    /** Case-insensitive comparison with another string. */
    bool equalsIgnoreCase (StringRef other) const noexcept;

    /** Case-insensitive comparison with another string. */
    bool equalsIgnoreCase (const char* other) const noexcept;

    /** Case-sensitive comparison with another string.
        @returns     0 if the two strings are identical; negative if this string comes before
                     the other one alphabetically, or positive if it comes after it.
    */
    int compare (const String& other) const noexcept;

    /** Case-sensitive comparison with another string.
        @returns     0 if the two strings are identical; negative if this string comes before
                     the other one alphabetically, or positive if it comes after it.
    */
    int compare (const char* other) const noexcept;

    /** Case-insensitive comparison with another string.
        @returns     0 if the two strings are identical; negative if this string comes before
                     the other one alphabetically, or positive if it comes after it.
    */
    int compareIgnoreCase (const String& other) const noexcept;

    /** Compares two strings, taking into account textual characteristics like numbers and spaces.

        This comparison is case-insensitive and can detect words and embedded numbers in the
        strings, making it good for sorting human-readable lists of things like filenames.

        @returns     0 if the two strings are identical; negative if this string comes before
                     the other one alphabetically, or positive if it comes after it.
    */
    int compareNatural (StringRef other, bool isCaseSensitive = false) const noexcept;

    /** Tests whether the string begins with another string.
        If the parameter is an empty string, this will always return true.
        Uses a case-sensitive comparison.
    */
    bool startsWith (StringRef text) const noexcept;

    /** Tests whether the string begins with a particular character.
        If the character is 0, this will always return false.
        Uses a case-sensitive comparison.
    */
    bool startsWithChar (water_uchar character) const noexcept;

    /** Tests whether the string begins with another string.
        If the parameter is an empty string, this will always return true.
        Uses a case-insensitive comparison.
    */
    bool startsWithIgnoreCase (StringRef text) const noexcept;

    /** Tests whether the string ends with another string.
        If the parameter is an empty string, this will always return true.
        Uses a case-sensitive comparison.
    */
    bool endsWith (StringRef text) const noexcept;

    /** Tests whether the string ends with a particular character.
        If the character is 0, this will always return false.
        Uses a case-sensitive comparison.
    */
    bool endsWithChar (water_uchar character) const noexcept;

    /** Tests whether the string ends with another string.
        If the parameter is an empty string, this will always return true.
        Uses a case-insensitive comparison.
    */
    bool endsWithIgnoreCase (StringRef text) const noexcept;

    /** Tests whether the string contains another substring.
        If the parameter is an empty string, this will always return true.
        Uses a case-sensitive comparison.
    */
    bool contains (StringRef text) const noexcept;

    /** Tests whether the string contains a particular character.
        Uses a case-sensitive comparison.
    */
    bool containsChar (water_uchar character) const noexcept;

    /** Tests whether the string contains another substring.
        Uses a case-insensitive comparison.
    */
    bool containsIgnoreCase (StringRef text) const noexcept;

    /** Tests whether the string contains another substring as a distinct word.

        @returns    true if the string contains this word, surrounded by
                    non-alphanumeric characters
        @see indexOfWholeWord, containsWholeWordIgnoreCase
    */
    bool containsWholeWord (StringRef wordToLookFor) const noexcept;

    /** Tests whether the string contains another substring as a distinct word.

        @returns    true if the string contains this word, surrounded by
                    non-alphanumeric characters
        @see indexOfWholeWordIgnoreCase, containsWholeWord
    */
    bool containsWholeWordIgnoreCase (StringRef wordToLookFor) const noexcept;

    /** Finds an instance of another substring if it exists as a distinct word.

        @returns    if the string contains this word, surrounded by non-alphanumeric characters,
                    then this will return the index of the start of the substring. If it isn't
                    found, then it will return -1
        @see indexOfWholeWordIgnoreCase, containsWholeWord
    */
    int indexOfWholeWord (StringRef wordToLookFor) const noexcept;

    /** Finds an instance of another substring if it exists as a distinct word.

        @returns    if the string contains this word, surrounded by non-alphanumeric characters,
                    then this will return the index of the start of the substring. If it isn't
                    found, then it will return -1
        @see indexOfWholeWord, containsWholeWordIgnoreCase
    */
    int indexOfWholeWordIgnoreCase (StringRef wordToLookFor) const noexcept;

    /** Looks for any of a set of characters in the string.
        Uses a case-sensitive comparison.

        @returns    true if the string contains any of the characters from
                    the string that is passed in.
    */
    bool containsAnyOf (StringRef charactersItMightContain) const noexcept;

    /** Looks for a set of characters in the string.
        Uses a case-sensitive comparison.

        @returns    Returns false if any of the characters in this string do not occur in
                    the parameter string. If this string is empty, the return value will
                    always be true.
    */
    bool containsOnly (StringRef charactersItMightContain) const noexcept;

    /** Returns true if this string contains any non-whitespace characters.

        This will return false if the string contains only whitespace characters, or
        if it's empty.

        It is equivalent to calling "myString.trim().isNotEmpty()".
    */
    bool containsNonWhitespaceChars() const noexcept;

    /** Returns true if the string matches this simple wildcard expression.

        So for example String ("abcdef").matchesWildcard ("*DEF", true) would return true.

        This isn't a full-blown regex though! The only wildcard characters supported
        are "*" and "?". It's mainly intended for filename pattern matching.
    */
    bool matchesWildcard (StringRef wildcard, bool ignoreCase) const noexcept;

    //==============================================================================
    // Substring location methods..

    /** Searches for a character inside this string.
        Uses a case-sensitive comparison.
        @returns    the index of the first occurrence of the character in this
                    string, or -1 if it's not found.
    */
    int indexOfChar (water_uchar characterToLookFor) const noexcept;

    /** Searches for a character inside this string.
        Uses a case-sensitive comparison.
        @param startIndex           the index from which the search should proceed
        @param characterToLookFor   the character to look for
        @returns            the index of the first occurrence of the character in this
                            string, or -1 if it's not found.
    */
    int indexOfChar (int startIndex, water_uchar characterToLookFor) const noexcept;

    /** Returns the index of the first character that matches one of the characters
        passed-in to this method.

        This scans the string, beginning from the startIndex supplied, and if it finds
        a character that appears in the string charactersToLookFor, it returns its index.

        If none of these characters are found, it returns -1.

        If ignoreCase is true, the comparison will be case-insensitive.

        @see indexOfChar, lastIndexOfAnyOf
    */
    int indexOfAnyOf (StringRef charactersToLookFor,
                      int startIndex = 0,
                      bool ignoreCase = false) const noexcept;

    /** Searches for a substring within this string.
        Uses a case-sensitive comparison.
        @returns    the index of the first occurrence of this substring, or -1 if it's not found.
                    If textToLookFor is an empty string, this will always return 0.
    */
    int indexOf (StringRef textToLookFor) const noexcept;

    /** Searches for a substring within this string.
        Uses a case-sensitive comparison.
        @param startIndex       the index from which the search should proceed
        @param textToLookFor    the string to search for
        @returns                the index of the first occurrence of this substring, or -1 if it's not found.
                                If textToLookFor is an empty string, this will always return -1.
    */
    int indexOf (int startIndex, StringRef textToLookFor) const noexcept;

    /** Searches for a substring within this string.
        Uses a case-insensitive comparison.
        @returns    the index of the first occurrence of this substring, or -1 if it's not found.
                    If textToLookFor is an empty string, this will always return 0.
    */
    int indexOfIgnoreCase (StringRef textToLookFor) const noexcept;

    /** Searches for a substring within this string.
        Uses a case-insensitive comparison.
        @param startIndex       the index from which the search should proceed
        @param textToLookFor    the string to search for
        @returns                the index of the first occurrence of this substring, or -1 if it's not found.
                                If textToLookFor is an empty string, this will always return -1.
    */
    int indexOfIgnoreCase (int startIndex, StringRef textToLookFor) const noexcept;

    /** Searches for a character inside this string (working backwards from the end of the string).
        Uses a case-sensitive comparison.
        @returns    the index of the last occurrence of the character in this string, or -1 if it's not found.
    */
    int lastIndexOfChar (water_uchar character) const noexcept;

    /** Searches for a substring inside this string (working backwards from the end of the string).
        Uses a case-sensitive comparison.
        @returns    the index of the start of the last occurrence of the substring within this string,
                    or -1 if it's not found. If textToLookFor is an empty string, this will always return -1.
    */
    int lastIndexOf (StringRef textToLookFor) const noexcept;

    /** Searches for a substring inside this string (working backwards from the end of the string).
        Uses a case-insensitive comparison.
        @returns    the index of the start of the last occurrence of the substring within this string, or -1
                    if it's not found. If textToLookFor is an empty string, this will always return -1.
    */
    int lastIndexOfIgnoreCase (StringRef textToLookFor) const noexcept;

    /** Returns the index of the last character in this string that matches one of the
        characters passed-in to this method.

        This scans the string backwards, starting from its end, and if it finds
        a character that appears in the string charactersToLookFor, it returns its index.

        If none of these characters are found, it returns -1.

        If ignoreCase is true, the comparison will be case-insensitive.

        @see lastIndexOf, indexOfAnyOf
    */
    int lastIndexOfAnyOf (StringRef charactersToLookFor,
                          bool ignoreCase = false) const noexcept;


    //==============================================================================
    // Substring extraction and manipulation methods..

    /** Returns the character at this index in the string.
        In a release build, no checks are made to see if the index is within a valid range, so be
        careful! In a debug build, the index is checked and an assertion fires if it's out-of-range.

        Also beware that depending on the encoding format that the string is using internally, this
        method may execute in either O(1) or O(n) time, so be careful when using it in your algorithms.
        If you're scanning through a string to inspect its characters, you should never use this operator
        for random access, it's far more efficient to call getCharPointer() to return a pointer, and
        then to use that to iterate the string.
        @see getCharPointer
    */
    water_uchar operator[] (int index) const noexcept;

    /** Returns the final character of the string.
        If the string is empty this will return 0.
    */
    water_uchar getLastCharacter() const noexcept;

    //==============================================================================
    /** Returns a subsection of the string.

        If the range specified is beyond the limits of the string, as much as
        possible is returned.

        @param startIndex   the index of the start of the substring needed
        @param endIndex     all characters from startIndex up to (but not including)
                            this index are returned
        @see fromFirstOccurrenceOf, dropLastCharacters, getLastCharacters, upToFirstOccurrenceOf
    */
    String substring (int startIndex, int endIndex) const;

    /** Returns a section of the string, starting from a given position.

        @param startIndex   the first character to include. If this is beyond the end
                            of the string, an empty string is returned. If it is zero or
                            less, the whole string is returned.
        @returns            the substring from startIndex up to the end of the string
        @see dropLastCharacters, getLastCharacters, fromFirstOccurrenceOf, upToFirstOccurrenceOf, fromLastOccurrenceOf
    */
    String substring (int startIndex) const;

    /** Returns a version of this string with a number of characters removed
        from the end.

        @param numberToDrop     the number of characters to drop from the end of the
                                string. If this is greater than the length of the string,
                                an empty string will be returned. If zero or less, the
                                original string will be returned.
        @see substring, fromFirstOccurrenceOf, upToFirstOccurrenceOf, fromLastOccurrenceOf, getLastCharacter
    */
    String dropLastCharacters (int numberToDrop) const;

    /** Returns a number of characters from the end of the string.

        This returns the last numCharacters characters from the end of the string. If the
        string is shorter than numCharacters, the whole string is returned.

        @see substring, dropLastCharacters, getLastCharacter
    */
    String getLastCharacters (int numCharacters) const;

    //==============================================================================
    /** Returns a section of the string starting from a given substring.

        This will search for the first occurrence of the given substring, and
        return the section of the string starting from the point where this is
        found (optionally not including the substring itself).

        e.g. for the string "123456", fromFirstOccurrenceOf ("34", true) would return "3456", and
                                      fromFirstOccurrenceOf ("34", false) would return "56".

        If the substring isn't found, the method will return an empty string.

        If ignoreCase is true, the comparison will be case-insensitive.

        @see upToFirstOccurrenceOf, fromLastOccurrenceOf
    */
    String fromFirstOccurrenceOf (StringRef substringToStartFrom,
                                  bool includeSubStringInResult,
                                  bool ignoreCase) const;

    /** Returns a section of the string starting from the last occurrence of a given substring.

        Similar to fromFirstOccurrenceOf(), but using the last occurrence of the substring, and
        unlike fromFirstOccurrenceOf(), if the substring isn't found, this method will
        return the whole of the original string.

        @see fromFirstOccurrenceOf, upToLastOccurrenceOf
    */
    String fromLastOccurrenceOf (StringRef substringToFind,
                                 bool includeSubStringInResult,
                                 bool ignoreCase) const;

    /** Returns the start of this string, up to the first occurrence of a substring.

        This will search for the first occurrence of a given substring, and then
        return a copy of the string, up to the position of this substring,
        optionally including or excluding the substring itself in the result.

        e.g. for the string "123456", upTo ("34", false) would return "12", and
                                      upTo ("34", true) would return "1234".

        If the substring isn't found, this will return the whole of the original string.

        @see upToLastOccurrenceOf, fromFirstOccurrenceOf
    */
    String upToFirstOccurrenceOf (StringRef substringToEndWith,
                                  bool includeSubStringInResult,
                                  bool ignoreCase) const;

    /** Returns the start of this string, up to the last occurrence of a substring.

        Similar to upToFirstOccurrenceOf(), but this finds the last occurrence rather than the first.
        If the substring isn't found, this will return the whole of the original string.

        @see upToFirstOccurrenceOf, fromFirstOccurrenceOf
    */
    String upToLastOccurrenceOf (StringRef substringToFind,
                                 bool includeSubStringInResult,
                                 bool ignoreCase) const;

    //==============================================================================
    /** Returns a copy of this string with any whitespace characters removed from the start and end. */
    String trim() const;

    /** Returns a copy of this string with any whitespace characters removed from the start. */
    String trimStart() const;

    /** Returns a copy of this string with any whitespace characters removed from the end. */
    String trimEnd() const;

    /** Returns a copy of this string, having removed a specified set of characters from its start.
        Characters are removed from the start of the string until it finds one that is not in the
        specified set, and then it stops.
        @param charactersToTrim     the set of characters to remove.
        @see trim, trimStart, trimCharactersAtEnd
    */
    String trimCharactersAtStart (StringRef charactersToTrim) const;

    /** Returns a copy of this string, having removed a specified set of characters from its end.
        Characters are removed from the end of the string until it finds one that is not in the
        specified set, and then it stops.
        @param charactersToTrim     the set of characters to remove.
        @see trim, trimEnd, trimCharactersAtStart
    */
    String trimCharactersAtEnd (StringRef charactersToTrim) const;

    //==============================================================================
    /** Returns an upper-case version of this string. */
    String toUpperCase() const;

    /** Returns an lower-case version of this string. */
    String toLowerCase() const;

    //==============================================================================
    /** Replaces a sub-section of the string with another string.

        This will return a copy of this string, with a set of characters
        from startIndex to startIndex + numCharsToReplace removed, and with
        a new string inserted in their place.

        Note that this is a const method, and won't alter the string itself.

        @param startIndex               the first character to remove. If this is beyond the bounds of the string,
                                        it will be constrained to a valid range.
        @param numCharactersToReplace   the number of characters to remove. If zero or less, no
                                        characters will be taken out.
        @param stringToInsert           the new string to insert at startIndex after the characters have been
                                        removed.
    */
    String replaceSection (int startIndex,
                           int numCharactersToReplace,
                           StringRef stringToInsert) const;

    /** Replaces all occurrences of a substring with another string.

        Returns a copy of this string, with any occurrences of stringToReplace
        swapped for stringToInsertInstead.

        Note that this is a const method, and won't alter the string itself.
    */
    String replace (StringRef stringToReplace,
                    StringRef stringToInsertInstead,
                    bool ignoreCase = false) const;

    /** Returns a string with all occurrences of a character replaced with a different one. */
    String replaceCharacter (water_uchar characterToReplace,
                             water_uchar characterToInsertInstead) const;

    /** Replaces a set of characters with another set.

        Returns a string in which each character from charactersToReplace has been replaced
        by the character at the equivalent position in newCharacters (so the two strings
        passed in must be the same length).

        e.g. replaceCharacters ("abc", "def") replaces 'a' with 'd', 'b' with 'e', etc.

        Note that this is a const method, and won't affect the string itself.
    */
    String replaceCharacters (StringRef charactersToReplace,
                              StringRef charactersToInsertInstead) const;

    /** Returns a version of this string that only retains a fixed set of characters.

        This will return a copy of this string, omitting any characters which are not
        found in the string passed-in.

        e.g. for "1122334455", retainCharacters ("432") would return "223344"

        Note that this is a const method, and won't alter the string itself.
    */
    String retainCharacters (StringRef charactersToRetain) const;

    /** Returns a version of this string with a set of characters removed.

        This will return a copy of this string, omitting any characters which are
        found in the string passed-in.

        e.g. for "1122334455", removeCharacters ("432") would return "1155"

        Note that this is a const method, and won't alter the string itself.
    */
    String removeCharacters (StringRef charactersToRemove) const;

    /** Returns a section from the start of the string that only contains a certain set of characters.

        This returns the leftmost section of the string, up to (and not including) the
        first character that doesn't appear in the string passed in.
    */
    String initialSectionContainingOnly (StringRef permittedCharacters) const;

    /** Returns a section from the start of the string that only contains a certain set of characters.

        This returns the leftmost section of the string, up to (and not including) the
        first character that occurs in the string passed in. (If none of the specified
        characters are found in the string, the return value will just be the original string).
    */
    String initialSectionNotContaining (StringRef charactersToStopAt) const;

    //==============================================================================
    /** Checks whether the string might be in quotation marks.

        @returns    true if the string begins with a quote character (either a double or single quote).
                    It is also true if there is whitespace before the quote, but it doesn't check the end of the string.
        @see unquoted, quoted
    */
    bool isQuotedString() const;

    /** Removes quotation marks from around the string, (if there are any).

        Returns a copy of this string with any quotes removed from its ends. Quotes that aren't
        at the ends of the string are not affected. If there aren't any quotes, the original string
        is returned.

        Note that this is a const method, and won't alter the string itself.

        @see isQuotedString, quoted
    */
    String unquoted() const;

    /** Adds quotation marks around a string.
        This will return a copy of the string with a quote at the start and end, (but won't
        add the quote if there's already one there, so it's safe to call this on strings that
        may already have quotes around them).
        Note that this is a const method, and won't alter the string itself.
        @param quoteCharacter   the character to add at the start and end
        @see isQuotedString, unquoted
    */
    String quoted (water_uchar quoteCharacter = '"') const;

    //==============================================================================
    /** Creates a string which is a version of a string repeated and joined together.

        @param stringToRepeat         the string to repeat
        @param numberOfTimesToRepeat  how many times to repeat it
    */
    static String repeatedString (StringRef stringToRepeat,
                                  int numberOfTimesToRepeat);

    /** Returns a copy of this string with the specified character repeatedly added to its
        beginning until the total length is at least the minimum length specified.
    */
    String paddedLeft (water_uchar padCharacter, int minimumLength) const;

    /** Returns a copy of this string with the specified character repeatedly added to its
        end until the total length is at least the minimum length specified.
    */
    String paddedRight (water_uchar padCharacter, int minimumLength) const;

    /** Creates a string from data in an unknown format.

        This looks at some binary data and tries to guess whether it's Unicode
        or 8-bit characters, then returns a string that represents it correctly.

        Should be able to handle Unicode endianness correctly, by looking at
        the first two bytes.
    */
    static String createStringFromData (const void* data, int size);

    /** Creates a String from a printf-style parameter list.

        I don't like this method. I don't use it myself, and I recommend avoiding it and
        using the operator<< methods or pretty much anything else instead. It's only provided
        here because of the popular unrest that was stirred-up when I tried to remove it...

        If you're really determined to use it, at least make sure that you never, ever,
        pass any String objects to it as parameters.
    */
    static String formatted (const String formatString, ... );

    //==============================================================================
    // Numeric conversions..

    /** Creates a string containing this signed 32-bit integer as a decimal number.
        @see getIntValue, getFloatValue, getDoubleValue, toHexString
    */
    explicit String (int decimalInteger);

    /** Creates a string containing this unsigned 32-bit integer as a decimal number.
        @see getIntValue, getFloatValue, getDoubleValue, toHexString
    */
    explicit String (unsigned int decimalInteger);

    /** Creates a string containing this signed 16-bit integer as a decimal number.
        @see getIntValue, getFloatValue, getDoubleValue, toHexString
    */
    explicit String (short decimalInteger);

    /** Creates a string containing this unsigned 16-bit integer as a decimal number.
        @see getIntValue, getFloatValue, getDoubleValue, toHexString
    */
    explicit String (unsigned short decimalInteger);

    /** Creates a string containing this signed 64-bit integer as a decimal number.
        @see getLargeIntValue, getFloatValue, getDoubleValue, toHexString
    */
    explicit String (int64 largeIntegerValue);

    /** Creates a string containing this unsigned 64-bit integer as a decimal number.
        @see getLargeIntValue, getFloatValue, getDoubleValue, toHexString
    */
    explicit String (uint64 largeIntegerValue);

    /** Creates a string containing this signed long integer as a decimal number.
        @see getIntValue, getFloatValue, getDoubleValue, toHexString
    */
    explicit String (long decimalInteger);

    /** Creates a string containing this unsigned long integer as a decimal number.
        @see getIntValue, getFloatValue, getDoubleValue, toHexString
    */
    explicit String (unsigned long decimalInteger);

    /** Creates a string representing this floating-point number.
        @param floatValue               the value to convert to a string
        @see getDoubleValue, getIntValue
    */
    explicit String (float floatValue);

    /** Creates a string representing this floating-point number.
        @param doubleValue              the value to convert to a string
        @see getFloatValue, getIntValue
    */
    explicit String (double doubleValue);

    /** Creates a string representing this floating-point number.
        @param floatValue               the value to convert to a string
        @param numberOfDecimalPlaces    if this is > 0, it will format the number using that many
                                        decimal places, and will not use exponent notation. If 0 or
                                        less, it will use exponent notation if necessary.
        @see getDoubleValue, getIntValue
    */
    String (float floatValue, int numberOfDecimalPlaces);

    /** Creates a string representing this floating-point number.
        @param doubleValue              the value to convert to a string
        @param numberOfDecimalPlaces    if this is > 0, it will format the number using that many
                                        decimal places, and will not use exponent notation. If 0 or
                                        less, it will use exponent notation if necessary.
        @see getFloatValue, getIntValue
    */
    String (double doubleValue, int numberOfDecimalPlaces);

    /** Reads the value of the string as a decimal number (up to 32 bits in size).

        @returns the value of the string as a 32 bit signed base-10 integer.
        @see getTrailingIntValue, getHexValue32, getHexValue64
    */
    int getIntValue() const noexcept;

    /** Reads the value of the string as a decimal number (up to 64 bits in size).
        @returns the value of the string as a 64 bit signed base-10 integer.
    */
    int64 getLargeIntValue() const noexcept;

    /** Parses a decimal number from the end of the string.

        This will look for a value at the end of the string.
        e.g. for "321 xyz654" it will return 654; for "2 3 4" it'll return 4.

        Negative numbers are not handled, so "xyz-5" returns 5.

        @see getIntValue
    */
    int getTrailingIntValue() const noexcept;

    /** Parses this string as a floating point number.

        @returns    the value of the string as a 32-bit floating point value.
        @see getDoubleValue
    */
    float getFloatValue() const noexcept;

    /** Parses this string as a floating point number.

        @returns    the value of the string as a 64-bit floating point value.
        @see getFloatValue
    */
    double getDoubleValue() const noexcept;

    /** Parses the string as a hexadecimal number.

        Non-hexadecimal characters in the string are ignored.

        If the string contains too many characters, then the lowest significant
        digits are returned, e.g. "ffff12345678" would produce 0x12345678.

        @returns    a 32-bit number which is the value of the string in hex.
    */
    int getHexValue32() const noexcept;

    /** Parses the string as a hexadecimal number.

        Non-hexadecimal characters in the string are ignored.

        If the string contains too many characters, then the lowest significant
        digits are returned, e.g. "ffff1234567812345678" would produce 0x1234567812345678.

        @returns    a 64-bit number which is the value of the string in hex.
    */
    int64 getHexValue64() const noexcept;

    /** Creates a string representing this 32-bit value in hexadecimal. */
    static String toHexString (int number);

    /** Creates a string representing this 64-bit value in hexadecimal. */
    static String toHexString (int64 number);

    /** Creates a string representing this 16-bit value in hexadecimal. */
    static String toHexString (short number);

    /** Creates a string containing a hex dump of a block of binary data.

        @param data         the binary data to use as input
        @param size         how many bytes of data to use
        @param groupSize    how many bytes are grouped together before inserting a
                            space into the output. e.g. group size 0 has no spaces,
                            group size 1 looks like: "be a1 c2 ff", group size 2 looks
                            like "bea1 c2ff".
    */
    static String toHexString (const void* data, int size, int groupSize = 1);

    //==============================================================================
    /** Returns the character pointer currently being used to store this string.

        Because it returns a reference to the string's internal data, the pointer
        that is returned must not be stored anywhere, as it can be deleted whenever the
        string changes.
    */
    inline CharPointerType getCharPointer() const noexcept      { return text; }

    /** Returns a pointer to a UTF-8 version of this string.

        Because it returns a reference to the string's internal data, the pointer
        that is returned must not be stored anywhere, as it can be deleted whenever the
        string changes.

        To find out how many bytes you need to store this string as UTF-8, you can call
        CharPointer_UTF8::getBytesRequiredFor (myString.getCharPointer())

        @see toRawUTF8, getCharPointer, toUTF16, toUTF32
    */
    CharPointer_UTF8 toUTF8() const;

   #ifdef CARLA_OS_WIN
    /** Convert string to UTF-16, Windows only */
    std::wstring toUTF16() const;
   #endif

    /** Returns a pointer to a UTF-8 version of this string.

        Because it returns a reference to the string's internal data, the pointer
        that is returned must not be stored anywhere, as it can be deleted whenever the
        string changes.

        To find out how many bytes you need to store this string as UTF-8, you can call
        CharPointer_UTF8::getBytesRequiredFor (myString.getCharPointer())

        @see getCharPointer, toUTF8, toUTF16, toUTF32
    */
    const char* toRawUTF8() const;

    /** */
    std::string toStdString() const;

    //==============================================================================
    /** Creates a String from a UTF-8 encoded buffer.
        If the size is < 0, it'll keep reading until it hits a zero.
    */
    static String fromUTF8 (const char* utf8buffer, int bufferSizeBytes = -1);

    /** Returns the number of bytes required to represent this string as UTF8.
        The number returned does NOT include the trailing zero.
        @see toUTF8, copyToUTF8
    */
    size_t getNumBytesAsUTF8() const noexcept;

    //==============================================================================
    /** Copies the string to a buffer as UTF-8 characters.

        Returns the number of bytes copied to the buffer, including the terminating null
        character.

        To find out how many bytes you need to store this string as UTF-8, you can call
        CharPointer_UTF8::getBytesRequiredFor (myString.getCharPointer())

        @param destBuffer       the place to copy it to; if this is a null pointer, the method just
                                returns the number of bytes required (including the terminating null character).
        @param maxBufferSizeBytes  the size of the destination buffer, in bytes. If the string won't fit, it'll
                                put in as many as it can while still allowing for a terminating null char at the
                                end, and will return the number of bytes that were actually used.
        @see CharPointer_UTF8::writeWithDestByteLimit
    */
    size_t copyToUTF8 (CharPointer_UTF8::CharType* destBuffer, size_t maxBufferSizeBytes) const noexcept;

    //==============================================================================
    /** Increases the string's internally allocated storage.

        Although the string's contents won't be affected by this call, it will
        increase the amount of memory allocated internally for the string to grow into.

        If you're about to make a large number of calls to methods such
        as += or <<, it's more efficient to preallocate enough extra space
        beforehand, so that these methods won't have to keep resizing the string
        to append the extra characters.

        @param numBytesNeeded   the number of bytes to allocate storage for. If this
                                value is less than the currently allocated size, it will
                                have no effect.
    */
    void preallocateBytes (size_t numBytesNeeded);

    /** Swaps the contents of this string with another one.
        This is a very fast operation, as no allocation or copying needs to be done.
    */
    void swapWith (String& other) noexcept;

    //==============================================================================
   #if 0 //def CARLA_OS_MAC
    /** OSX ONLY - Creates a String from an OSX CFString. */
    static String fromCFString (CFStringRef cfString);

    /** OSX ONLY - Converts this string to a CFString.
        Remember that you must use CFRelease() to free the returned string when you're
        finished with it.
    */
    CFStringRef toCFString() const;
   #endif

   #ifdef CARLA_OS_MAC
    /** OSX ONLY - Returns a copy of this string in which any decomposed unicode characters have
        been converted to their precomposed equivalents. */
    String convertToPrecomposedUnicode() const;
   #endif

    /** Returns the number of String objects which are currently sharing the same internal
        data as this one.
    */
    int getReferenceCount() const noexcept;

private:
    //==============================================================================
    CharPointerType text;

    //==============================================================================
    struct PreallocationBytes
    {
        explicit PreallocationBytes (size_t) noexcept;
        size_t numBytes;
    };

    explicit String (const PreallocationBytes&); // This constructor preallocates a certain amount of memory
    size_t getByteOffsetOfEnd() const noexcept;
};

//==============================================================================
/** Concatenates two strings. */
String operator+ (const char* string1, const String& string2);
/** Concatenates two strings. */
String operator+ (char string1, const String& string2);
/** Concatenates two strings. */
String operator+ (water_uchar string1, const String& string2);

/** Concatenates two strings. */
String operator+ (String string1, const String& string2);
/** Concatenates two strings. */
String operator+ (String string1, const char* string2);
/** Concatenates two strings. */
String operator+ (String string1, char characterToAppend);
/** Concatenates two strings. */
String operator+ (String string1, water_uchar characterToAppend);

//==============================================================================
/** Appends a character at the end of a string. */
String& operator<< (String& string1, char characterToAppend);
/** Appends a character at the end of a string. */
String& operator<< (String& string1, water_uchar characterToAppend);

/** Appends a string to the end of the first one. */
String& operator<< (String& string1, const char* string2);
/** Appends a string to the end of the first one. */
String& operator<< (String& string1, const String& string2);
/** Appends a string to the end of the first one. */
String& operator<< (String& string1, StringRef string2);

/** Appends a decimal number at the end of a string. */
String& operator<< (String& string1, short number);
/** Appends a decimal number at the end of a string. */
String& operator<< (String& string1, int number);
/** Appends a decimal number at the end of a string. */
String& operator<< (String& string1, long number);
/** Appends a decimal number at the end of a string. */
String& operator<< (String& string1, int64 number);
/** Appends a decimal number at the end of a string. */
String& operator<< (String& string1, uint64 number);
/** Appends a decimal number at the end of a string. */
String& operator<< (String& string1, float number);
/** Appends a decimal number at the end of a string. */
String& operator<< (String& string1, double number);

//==============================================================================
/** Case-sensitive comparison of two strings. */
bool operator== (const String& string1, const String& string2) noexcept;
/** Case-sensitive comparison of two strings. */
bool operator== (const String& string1, const char* string2) noexcept;
/** Case-sensitive comparison of two strings. */
bool operator== (const String& string1, const CharPointer_UTF8 string2) noexcept;

/** Case-sensitive comparison of two strings. */
bool operator!= (const String& string1, const String& string2) noexcept;
/** Case-sensitive comparison of two strings. */
bool operator!= (const String& string1, const char* string2) noexcept;
/** Case-sensitive comparison of two strings. */
bool operator!= (const String& string1, const CharPointer_UTF8 string2) noexcept;

/** Case-sensitive comparison of two strings. */
bool operator>  (const String& string1, const String& string2) noexcept;
/** Case-sensitive comparison of two strings. */
bool operator<  (const String& string1, const String& string2) noexcept;
/** Case-sensitive comparison of two strings. */
bool operator>= (const String& string1, const String& string2) noexcept;
/** Case-sensitive comparison of two strings. */
bool operator<= (const String& string1, const String& string2) noexcept;

//==============================================================================
/** This operator allows you to write a water String directly to std output streams.
    This is handy for writing strings to std::cout, std::cerr, etc.
*/
template <class traits>
std::basic_ostream <char, traits>& operator<< (std::basic_ostream <char, traits>& stream, const String& stringToWrite)
{
    return stream << stringToWrite.toRawUTF8();
}

/** Writes a string to an OutputStream as UTF8. */
OutputStream& operator<< (OutputStream& stream, const String& stringToWrite);

/** Writes a string to an OutputStream as UTF8. */
OutputStream& operator<< (OutputStream& stream, StringRef stringToWrite);

//==============================================================================
struct StartEndString {
    StartEndString (String::CharPointerType s, String::CharPointerType e) noexcept : start (s), end (e) {}
    operator String() const   { return String (start, end); }
    String::CharPointerType start, end;
};

}

#include "StringRef.h"

#endif // WATER_STRING_H_INCLUDED
