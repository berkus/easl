/**
*   \file   slow_string.h
*   \author Dave Reid
*   \brief  Slow string implementation of the string class.
*
*   The slow string implementation is essentially a wrapper around a C-style string.
*   It contains only a single property - a pointer to the content of the string. This
*   implementation is designed to keep memory consumption to a minumum.
*
*   Internally the class uses strlen(), so encoding '\0' anywhere in the string other
*   than the end will cause the string to become corrupted as soon as it is modified.
*   If the string remains static, however, a NULL character can be encoded by modifying
*   the pointer directly from outside the class. This problem only affects slow_string.
*   fast_string is not affected by this problem.
*/
#ifndef __EASL_SLOW_STRING_H_
#define __EASL_SLOW_STRING_H_

#include "strlen.h"
#include "strcpy.h"
#include "strconvertsize.h"
#include "strgetchar.h"
#include "strequal.h"

namespace easl
{

template <typename T>
class slow_string
{
public:

    // Note: In the constructors, it's important that m_data is assigned to NULL before
    // assigning the string. This is because assign() always attempts to delete m_data.
    // Not initialising it to NULL will cause a crash.

    /**
    *   \brief  Default constructor.
    */
    slow_string() : m_data(NULL)
    {
        this->assign((const T *)NULL);
    }

    /**
    *   \brief              Constructor.
    *   \param  str [in]    The string to initialise this string to.
    */
#ifdef EASL_OPTION_NO_GENERIC_ASSIGN
    slow_string(const T *str) : m_data(NULL)
    {
        this->assign(str);
    }
#else
    template <typename U>
    slow_string(const U *str) : m_data(NULL)
    {
        this->assign(str);
    }
#endif

    /**
    *   \brief              Constructor.
    *   \param  str [in]    The string to initialise this string to.
    */
    slow_string(const slow_string<T> &str) : m_data(NULL)
    {
        this->assign(str.c_str());
    }

#ifndef EASL_OPTION_NO_GENERIC_ASSIGN
    template <typename U>
    slow_string(const slow_string<U> &str) : m_data(NULL)
    {
        this->assign(str.c_str());
    }
#endif


    /**
    *   \brief  Destructor.
    */
    virtual ~slow_string()
    {
        delete [] this->m_data;
    }



    /**
    *   \brief  Retrieves a reference to the pointer to the contents of the string.
    *   \return A reference to the pointer to the contents of the string.
    *
    *   \remarks
    *       The returned pointer is always NULL terminated.
    *       \par
    *       Because a reference is being returned, a pointer to the pointer can be retrieved with
    *       the address-of operator.
    */
    const T * const & c_str() const
    {
        return this->m_data;
    }

    /**
    *   \brief  Retrieves a reference to the pointer to the contents of the string.
    *   \return A reference to the pointer to the contents of the string.
    *
    *   \remarks
    *       This version is specific to slow strings. It returns a non-constant pointer so that
    *       the internal string can be manipulated from outside the class. The returned pointer
    *       is always NULL terminated as usual.
    *       \par
    *       Because a reference is being returned, a pointer to the pointer can be retrieved with
    *       the address-of operator.
    */
    T *& c_str()
    {
        return this->m_data;
    }

    /**
    *   \brief              Assigns another string to this string.
    *   \param  str [in]    The new string to assign to this string.
    *   \param  len [in]    The length of the input string, or -1 if the length is unknown.
    *   \return             A reference to this string.
    *
    *   \remarks
    *       If the length of the input string is known, set \c len to the length of the string.
    *       This will prevent this string from manually retrieving the length of the string.
    */
#ifdef EASL_OPTION_NO_GENERIC_ASSIGN
    slow_string<T> & assign(const T *str, size_t len = 0)
    {
        if (this->m_data == (const T *)str && this->m_data != NULL)
        {
            return *this;
        }

        // No matter what happens, the data must be deallocated. Therefore, we will do it first.
        delete [] m_data;

        // If the input string is NULL, we will set this string to an empty string.
        if (str == NULL)
        {
            this->m_data = new T[1];
            this->m_data[0] = NULL;
        }
        else
        {
            if (len == 0)
            {
                len = easl::strlen(str);
            }

            // Now we just need to copy the string over.
            this->m_data = new T[len + 1];
            easl::strcpy(this->m_data, str, len + 1);
        }

        return *this;
    }
#else
    template <typename U>
    slow_string<T> & assign(const U *str, size_t len = 0)
    {
        if (this->m_data == (const T *)str && this->m_data != NULL)
        {
            return *this;
        }

        // No matter what happens, the data must be deallocated. Therefore, we will do it first.
        delete [] m_data;

        // If the input string is NULL, we will set this string to an empty string.
        if (str == NULL)
        {
            this->m_data = new T[1];
            this->m_data[0] = NULL;
        }
        else
        {
            if (len == 0)
            {
                len = easl::strconvertsize<T>(str);
            }

            // All we're really doing is a simple conversion.
            this->m_data = new T[len];
            easl::strconvert(this->m_data, str);
        }

        return *this;
    }
#endif


    /**
    *   \brief              Helper method for appending a string to this string.
    *   \param  str [in]    The string to append to this string.
    *   \param  len [in]    The length in U's of \c str.
    *   \return             A reference to this string.
    *
    *   \remarks
    *       The \c len parameter should specify the number of U's in \c str and not
    *       the number of characters.
    */
#ifdef EASL_OPTION_NO_GENERIC_MANIPULATION
    slow_string<T> & append(const T *str, size_t len = 0)
    {
        if (str != NULL)
        {
            // First we need to determine how much extra space we need to allocate to append
            // the new string.
            if (len == 0)
            {
                len = easl::strlen(str);
            }

            // Now we need to find the length of this string.
            size_t this_size = this->length();

            // Now grab our current pointer so we can copy it into our new memory space later.
            T *old_data = this->m_data;

            // Now we can allocate some more memory.
            this->m_data = new T[this_size + len + 1];
            
            // Copy our old data back into the string.
            easl::strcpy(this->m_data, old_data, this_size + len + 1);

            // Now we copy our input string into our new buffer.
            easl::strcpy(this->m_data + this_size, str, len + 1);

            // Delete our previous data.
            delete [] old_data;
        }

        return *this;
    }
#else
    template <typename U>
    slow_string<T> & append(const U *str, size_t len = 0)
    {
        if (str != NULL)
        {
            // First we need to determine how much extra space we need to allocate to append
            // the new string.
            size_t convert_size = easl::strconvertsize<T>(str);

            // Now we need to find the length of this string.
            size_t this_size = this->length();

            // Now grab our current pointer so we can copy it into our new memory space later.
            T *old_data = this->m_data;

            // Now we can allocate some more memory.
            this->m_data = new T[this_size + convert_size];
            
            // Copy our old data back into the string.
            memcpy(this->m_data, old_data, sizeof(T) * this_size);

            // Now we copy our input string into our new buffer.
            easl::strconvert(this->m_data + this_size, str);

            // Delete our previous data.
            delete [] old_data;
        }

        return *this;
    }
#endif

    /**
    *   \brief                  Appends an individual character to the string
    *   \param  character [in]  The character to append.
    *   \return                 A reference to this string.
    */
    slow_string<T> & append(uchar32_t character)
    {
        // First we need to determine how many T's to increase the buffer by.
        size_t added_size = easl::get_char_size<T>(character);

        // We also need the length of this string.
        size_t this_size = this->length();

        // Now grab our current pointer so we can copy it into our new memory space later.
        T *old_data = this->m_data;

        // Now we can allocate some more memory.
        this->m_data = new T[this_size + added_size + 1];
        
        // Copy our old data back into the string.
        easl::strcpy(this->m_data, old_data, this_size + added_size + 1);

        // Now we need to write this character to the string.
        easl::write_char(this->m_data + this_size, character, added_size);

        // Null terminate the string.
        this->m_data[this_size + added_size] = 0;

        // Delete our previous data.
        delete [] old_data;

        return *this;
    }

    /**
    *   \brief  Retrieves the length of this string.
    *   \return The length of this string.
    *
    *   \remarks
    *       This function will return the number of T's in the string, and not necessarily the
    *       number of characters.
    */
    size_t length() const
    {
        return easl::strlen(this->m_data);
    }


    /**
    *   \brief  Determines if the strings contains any characters.
    *   \return True if the string does not contain any characters; false otherwise.
    */
    bool empty() const
    {
        if (this->m_data != NULL)
        {
            return this->m_data[0] == NULL;
        }

        return true;
    }


    /**
    *   \brief              Retrieves a character from the string.
    *   \param  index [in]  The index of the character to retrieve.
    *   \return             The character if the index is valid, zero otherwise.
    *
    *   \remarks
    *       For strings with variable length characters, this method can be quite slow
    *       as it must iterate through each character. We should probably try to come
    *       up with a quicker way of doing this...
    */
    uchar32_t getchar(size_t index)
    {
        assert(this->length() < index);

        return easl::strgetchar(this->m_data, index);
    }


    /**
    *   \brief              Assignment operator.
    *   \param  str [in]    The string to assign to this string.
    *   \return             A reference to this string.
    */
#ifdef EASL_OPTION_NO_GENERIC_ASSIGN
    slow_string<T> & operator =(const T *str)
    {
        return this->assign(str);
    }
    slow_string<T> & operator =(const slow_string<T> &str)
    {
        return this->assign(str.c_str());
    }
#else
    template <typename U>
    slow_string<T> & operator =(const U *str)
    {
        return this->assign(str);
    }

    /// \copydoc    slow_string::operator =(const U *)
    template <typename U>
    slow_string<T> & operator =(const slow_string<U> &str)
    {
        return this->assign(str.c_str());
    }
#endif


    /**
    *   \brief              Comparison operator.
    *   \param  str [in]    The string to compare this string siwth.
    *   \return             True if the two strings are equal; false otherwise.
    *
    *   \remarks
    *       The comparison is case sensitive. "Some String" does not equal "some string".
    */
#ifdef EASL_OPTION_NO_GENERIC_COMPARISON
    bool operator ==(const T *str) const
    {
        return easl::strequal(this->m_data, str);
    }
    bool operator ==(const slow_string<T> &str) const
    {
        return easl::strequal(this->m_data, str.c_str());
    }
#else
    template <typename U>
    bool operator ==(const U *str) const
    {
        return easl::strequal(this->m_data, str);
    }

    /// \copydoc    slow_string::operator ==(const U *) const
    template <typename U>
    bool operator ==(const slow_string<U> &str) const
    {
        return easl::strequal(this->m_data, str.c_str());
    }
#endif


    /**
    *   \brief              Appends a string to this string.
    *   \param  str [in]    The string to append to this string.
    *   \return             A reference to this string.
    */
#ifdef EASL_OPTION_NO_GENERIC_MANIPULATION
    slow_string<T> & operator +=(const T *str)
    {
        return this->append(str);
    }
    slow_string<T> & operator +=(const slow_string<T> &str)
    {
        return this->append(str.c_str());
    }
#else
    template <typename U>
    slow_string<T> & operator +=(const U *str)
    {
        return this->append(str);
    }

    /// \copydoc    slow_string::operator +=(const U *)
    template <typename U>
    slow_string<T> & operator +=(const slow_string<U> &str)
    {
        return this->append(str.c_str());
    }
#endif

    /**
    *   \brief                  Appends a character to this string.
    *   \param  character [in]  The character to append.
    *   \return                 A reference to this string.
    */
    slow_string<T> & operator +=(uchar32_t character)
    {
        return this->append(character);
    }


    /**
    *   \brief              Creates a new string equal to another string appended to this string.
    *   \param  str [in]    The string to be appended to the end of the new string.
    *   \return             A new string equal to this string with the input string appeneded to the end.
    */
#ifdef EASL_OPTION_NO_GENERIC_MANIPULATION
    slow_string<T> operator +(const T *str) const
    {
        slow_string<T> new_str(*this);
        return new_str += str;
    }
    slow_string<T> operator +(const slow_string<T> &str) const
    {
        slow_string<T> new_str(*this);
        return new_str += str;
    }
#else
    template <typename U>
    slow_string<T> operator +(const U *str) const
    {
        slow_string<T> new_str(*this);
        return new_str += str;
    }

    /// \copydoc    slow_string::operator +(const U *)
    template <typename U>
    slow_string<T> operator +(const slow_string<U> &str) const
    {
        slow_string<T> new_str(*this);
        return new_str += str;
    }
#endif

    /**
    *   \brief                  Creates a new string equal to a character appended to this string.
    *   \param  character [in]  The character to be appended to the end of the new string.
    *   \return                 A new string equal to this string with the input character appeneded to the end.
    */
    slow_string<T> operator +(uchar32_t character) const
    {
        slow_string<T> new_str(*this);
        return new_str += str;
    }



    /**
    *   \brief  Casting operator to a const T *.
    */
    operator const T *() const
    {
        return this->m_data;
    }

    /**
    *   \brief  Casting operator to a T *.
    */
    operator T *()
    {
        return this->m_data;
    }



private:

    /// Pointer to the content of the string.
    T *m_data;

};

}

#endif // __EASL_SLOW_STRING_H_