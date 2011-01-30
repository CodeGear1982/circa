// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "circa.h"

namespace circa {
namespace if_block_function {

    void formatSource(StyledSource* source, Term* term)
    {
        Branch& contents = term->nestedContents;

        for (int branch_index=0; branch_index < contents.length(); branch_index++) {
            Term* branch_term = contents[branch_index];

            if (is_hidden(branch_term))
                continue;

            append_phrase(source,
                    branch_term->stringPropOptional("syntax:preWhitespace", ""),
                    branch_term, token::WHITESPACE);

            if (branch_index == 0) {
                append_phrase(source, "if ", branch_term, phrase_type::KEYWORD);
                format_source_for_input(source, branch_term, 0);
            } else if (branch_index < (contents.length()-2)) {
                append_phrase(source, "elif ", branch_term, phrase_type::KEYWORD);
                format_source_for_input(source, branch_term, 0);
            }
            else
                append_phrase(source, "else", branch_term, phrase_type::UNDEFINED);

            format_branch_source(source, branch_term->nestedContents, branch_term);
        }

        //append_phrase(source, term->stringPropOptional("syntax:whitespaceBeforeEnd", ""),
        //        term, token::WHITESPACE);
        //append_phrase(source, "end", term, phrase_type::KEYWORD);
    }

    int get_register_count(Term* term)
    {
        Branch& contents = term->nestedContents;

        // check if term is still being initialized:
        if (contents.length() == 0)
            return 1;

        Branch& outerRebinds = contents[contents.length()-1]->nestedContents;
        return outerRebinds.length() + 1;
    }

    void setup(Branch& kernel)
    {
        IF_BLOCK_FUNC = import_function(kernel, evaluate_if_block, "if_block() -> any");
        get_function_attrs(IF_BLOCK_FUNC)->formatSource = formatSource;
        get_function_attrs(IF_BLOCK_FUNC)->getRegisterCount = get_register_count;
        function_t::set_exposed_name_path(IF_BLOCK_FUNC, "#joining");
    }
}
}
