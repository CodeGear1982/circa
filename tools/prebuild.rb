#!/usr/bin/env ruby

def each_cpp_file(dir)
    Dir.foreach(dir) do |path|
        if path.end_with?('.cpp')
            yield path
        end
    end
end

def get_cpp_file_names(dir)
    out = []
    each_cpp_file(dir) do |path|
        path =~ /(.*?)\.cpp/
        out << $1
    end
    out
end

def setup_builtin_functions(dir)
    namespaces = get_cpp_file_names(dir).map { |n| "#{n}_function" }.sort
<<-eos
// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

// This file is generated during the build process by prebuild.rb .
// You should probably not edit this file manually.

#include "common_headers.h"

#include "branch.h"

namespace circa {

#{namespaces.map { |ns| "namespace #{ns} { void setup(Branch& kernel); }" } * "\n"}

void setup_builtin_functions(Branch& kernel)
{
    #{namespaces.map { |ns| "#{ns}::setup(kernel);" } * "\n    "}
}

} // namespace circa
eos
end

def register_all_tests(dir)
    namespaces = get_cpp_file_names(dir)
<<-eos
// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

// This file is generated during the build process by prebuild.rb .
// You should probably not edit this file manually.

#include "common_headers.h"

#include "testing.h"

namespace circa {

#{namespaces.map { |ns| "namespace #{ns} { void register_tests(); }" } * "\n"}

void register_all_tests()
{
    gTestCases.clear();

    #{namespaces.map { |ns| "#{ns}::register_tests();" } * "\n    "}
}

} // namespace circa
eos
end

def include_cpps(baseDir, targetDir)
    out = []
    each_cpp_file(File.join(baseDir,targetDir)) do |path|
        out << "#include \"#{File.join(targetDir,path)}\""
    end
    out * "\n" + "\n"
end

def circa_source_to_c(file, variableName)
    quoted_file = []
    def escape_line(line)
        out = ""
        line.each_char { |c|
            if c == '"'
                out << "\\\""
            elsif c == '\\'
                out << "\\"
            else
                out << c
            end
        }
        out
    end
    File.new(file).each do |line|
        line = line.slice(0,line.length-1)
        line = escape_line(line)
        quoted_file << "\"#{line}\\n\""
    end

<<-eos
// This file was autogenerated from #{file}

namespace circa {

const char* #{variableName} = #{quoted_file * "\n    "};

} // namespace circa
eos
end

if not File.exists?('src/generated') then Dir.mkdir('src/generated') end
File.new('src/generated/setup_builtin_functions.cpp', 'w').write(
        setup_builtin_functions('src/builtin_functions'))
File.new('src/generated/register_all_tests.cpp', 'w').write(
        register_all_tests('src/tests'))
File.new('src/generated/all_tests.cpp', 'w').write(
        include_cpps('src','tests'))
File.new('src/generated/all_builtin_functions.cpp', 'w').write(
        include_cpps('src','builtin_functions'))
File.new('src/generated/all_builtin_types.cpp', 'w').write(
        include_cpps('src','builtin_types'))
File.new('src/generated/builtin_script_text.cpp', 'w').write(
        circa_source_to_c('src/ca/builtins.ca', 'BUILTIN_SCRIPT_TEXT'))

