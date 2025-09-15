#include <Variable.h>
#include <StringUtil.h>

char gStoredLine[256];

bool ReadTokens(u32& whitespace, char* token_a, char* token_b, char* line)
{
    whitespace = 0;
    *token_a = '\0';
    *token_b = '\0';

    while (true)
    {
        char c = *line;
        if (c == '\0') return false;
        if (!IsWhiteSpace(c)) break;
        if (c == '\t') whitespace++;
        line++;
    }

    while (true)
    {
        char c = *line;
        if (c == '\0' || IsWhiteSpace(c)) break;
        *token_a++ = c;
        line++;
    }

    *token_a = '\0';

    if (*line == '\0') return true;
    line++;
    if (*line == '"') line++;

    while (true)
    {
        char c = *line++;
        if (c == '\0' || c == '"' || IsWhiteSpace(c)) 
            break;
        *token_b++ = c;
    }

    *token_b = '\0';
    return true;
}

bool ReadLine(char* line, ByteArray& vec, u32& offset)
{
    if (*gStoredLine != '\0')
    {
        strcpy(line, gStoredLine);
        *gStoredLine = '\0';
        return true;
    }

    int len = 0;
    while (offset < vec.size())
    {
        char c = vec[offset++];
        if (c == '\0' || c == '\n') break;
        if (c == '\r' || len >= 255) continue;
        line[len++] = c;
    }
    
    line[len] = '\0';
    return len != 0 || vec.size() > offset;
}

ReflectReturn LoadVariablesChildren(ByteArray& vec, u32& offset, CGatherVariables& variables, u32 depth)
{
    if (variables.Type != VARIABLE_TYPE_RESOURCEPTR)
    {
        variables.Children.clear();
        if (variables.ReflectFunction != NULL)
        {
            ReflectReturn ret = variables.ReflectFunction(variables, variables.Data);
            if (ret != REFLECT_NOT_IMPLEMENTED && ret != REFLECT_OK)
                return ret;
        }
    }


    char line[256];
    char token_a[256];
    char token_b[256];

    while (ReadLine(line, vec, offset))
    {
        u32 whitespace;
        if (!ReadTokens(whitespace, token_a, token_b, line)) continue;

        if (whitespace < depth)
        {
            strcpy(gStoredLine, line);
            break;
        }

        if (depth < whitespace)
        {
            MMLogCh(DC_RESOURCE, "!! Parser ignoring child variable: {%s}\n", token_a);
            continue;
        }

        if (*token_a == '[')
        {
            int cpos = strlen(token_a) - 1;
            token_a[cpos] = '\0';
            int index = strtol(token_a + 1, NULL, 10);
            token_a[cpos] = ']';

            if (index < variables.Children.size())
            {
                CGatherVariables& child = variables.Children[index];
                if (*token_b != '\0')
                    child.SetString(token_b);
                LoadVariablesChildren(vec, offset, child, depth + 1);
            }
            else MMLogCh(DC_RESOURCE, "!! Parser ignoring variable: {%s}\n", token_a);
        }
        else
        {
            for (int i = 0; i < variables.Children.size(); ++i)
            {
                CGatherVariables& child = variables.Children[i];
                if (child.Name != NULL && StringCompare(child.Name, token_a) == 0)
                {
                    if (*token_b != '\0')
                        child.SetString(token_b);
                    LoadVariablesChildren(vec, offset, child, depth + 1);
                    break;
                }
            }
        }
    }

    variables.Children.clear();
    return REFLECT_OK;
}

ReflectReturn GatherVariablesLoad(ByteArray& v, CGatherVariables& variables, bool ignore_head, char* header_4bytes) // 752
{
    variables.Purpose = GATHER_TYPE_LOAD;
    variables.Visited = new std::map<void*, void*>();
    
    u32 offset = 0;
    if (header_4bytes != NULL)
    {
        offset = 4;
    }

    if (ignore_head)
        return LoadVariablesChildren(v, offset, variables, 0);

    return REFLECT_NOT_IMPLEMENTED;
}

ReflectReturn SaveVariablesChildren(ByteArray& f, CGatherVariables& variables, u32 depth, MMString<char>& scratch);

ReflectReturn SaveVariable(ByteArray& f, CGatherVariables& variables, u32 depth, u32 index, MMString<char>& scratch)
{
    scratch.resize(0, '\0');
    if (variables.Data != NULL)
    {
        for (u32 space = 0; space < depth; ++space) scratch += '\t';

        char s[256];
        bool valid = variables.GetString(s);

        if (variables.HasName())
        {
            scratch += variables.GetName();
        }
        else
        {
            char array[32];
            sprintf(array, "[%d]", index);
            scratch += array;
        }

        if (valid)
        {
            scratch += '\t';
            scratch += s;
        }
    }

    scratch += '\n';
    // supposed to use insert, but for now we're doing this
    // because i dont want to implement the function for insertion ranges
    for (int i = 0; i < scratch.size(); ++i)
        f.push_back(*(scratch.c_str() + i));
    
    return SaveVariablesChildren(f, variables, depth + 1, scratch);
}

ReflectReturn SaveVariablesChildren(ByteArray& f, CGatherVariables& variables, u32 depth, MMString<char>& scratch)
{
    // For some reason the save function is missing the check
    // for if the variable type is RESOURCEPTR, probably because it's not used normally?
    // But either way, adding it in otherwise it serialises dead stack pointers.
    ReflectReturn rv;
    if (variables.Type  != VARIABLE_TYPE_RESOURCEPTR)
    {
        variables.Children.clear();
        if (variables.ReflectFunction != NULL)
        {
            rv = variables.ReflectFunction(variables, variables.Data);
            if (rv != REFLECT_NOT_IMPLEMENTED && rv != REFLECT_OK)
                return rv;
        }
    }
    
    for (u32 i = 0; i < variables.Children.size(); ++i)
    {
        if ((rv = SaveVariable(f, variables.Children[i], depth, i, scratch)) != REFLECT_OK)
            return rv;
    }

    variables.Children.clear();
    return REFLECT_OK;
}

ReflectReturn GatherVariablesSave(ByteArray& v, CGatherVariables& variables, bool ignore_head, const char* header_4bytes)
{
    variables.Purpose = GATHER_TYPE_SAVE;
    variables.Visited = new std::map<void*, void*>();
    v.resize(0);

    if (header_4bytes != NULL)
    {
        for (int i = 0; i < 4; ++i)
            v.push_back(header_4bytes[i]);
        v.push_back('\n');
    }

    MMString<char> scratch;
    if (ignore_head) 
        return SaveVariablesChildren(v, variables, 0, scratch);
    return SaveVariable(v, variables, 0, 0, scratch);
}