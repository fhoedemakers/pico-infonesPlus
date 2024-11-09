#include "FrensHelpers.h"

namespace Frens
{
    // string helper functions
    //
    // test if string ends with suffix
    //
    bool endsWith(std::string const &str, std::string const &suffix)
    {
        if (str.length() < suffix.length())
        {
            return false;
        }
        return str.compare(str.length() - suffix.length(), suffix.length(), suffix) == 0;
    }
    //
    // returns lowercase of string s
    //
    std::string str_tolower(std::string s)
    {
        std::transform(s.begin(), s.end(), s.begin(),
                       [](unsigned char c)
                       { return std::tolower(c); } // correct
        );
        return s;
    }

    bool cstr_endswith(const char *string, const char *width)
    {
        int lstring = strlen(string);
        int wlen = strlen(width);
        if (wlen >= lstring)
        {
            return false;
        }
        int pos = lstring - wlen;
        return (strcmp(string + pos, width) == 0);
    }
#define INITIAL_CAPACITY 10

    char **cstr_split(const char *str, const char *delimiters, int *count)
    {
        if (str == NULL || delimiters == NULL)
        {
            *count = 0;
            return NULL;
        }

        // Create a modifiable copy of the input string
        char *str_copy = strdup(str);
        if (str_copy == NULL)
        {
            *count = 0;
            return NULL;
        }

        // Initial memory allocation for the result array
        int capacity = INITIAL_CAPACITY;
        char **result = (char **)malloc(capacity * sizeof(char *));
        if (result == NULL)
        {
            free(str_copy);
            *count = 0;
            return NULL;
        }

        *count = 0;
        char *token = strtok(str_copy, delimiters);
        while (token != NULL)
        {
            // Skip empty tokens
            if (*token != '\0')
            {
                // Reallocate if necessary
                if (*count >= capacity)
                {
                    capacity *= 2;
                    char **temp = (char **)realloc(result, capacity * sizeof(char *));
                    if (temp == NULL)
                    {
                        // Memory allocation failed, clean up
                        for (int i = 0; i < *count; ++i)
                        {
                            free(result[i]);
                        }
                        free(result);
                        free(str_copy);
                        *count = 0;
                        return NULL;
                    }
                    result = temp;
                }

                // Allocate memory for the token and copy it
                result[*count] = strdup(token);
                if (result[*count] == NULL)
                {
                    // Memory allocation failed, clean up
                    for (int i = 0; i < *count; ++i)
                    {
                        free(result[i]);
                    }
                    free(result);
                    free(str_copy);
                    *count = 0;
                    return NULL;
                }
                (*count)++;
            }
            token = strtok(NULL, delimiters);
        }

        free(str_copy);
        return result;
    }
}