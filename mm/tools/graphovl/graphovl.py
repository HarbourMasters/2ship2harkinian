#!/usr/bin/env python3
#
# Generates graphs for actor overlay files
#

import argparse, os, re, sys
from configparser import ConfigParser

try:
    from graphviz import Digraph
except ModuleNotFoundError:
    print("Module 'graphviz' is not installed", file=sys.stderr)
    print("You can install it using: pip3 install graphviz", file=sys.stderr)
    print("You may also need to install it on your system", file=sys.stderr)
    print("On Debian/Ubuntu derivates you can use: apt install graphviz", file=sys.stderr)
    sys.exit(1)

script_dir = os.path.dirname(os.path.realpath(__file__))
config = ConfigParser()

func_names = list()
func_definitions = list()
line_numbers_of_functions = list()

# Make actor source file path from actor name
def actor_src_path(name):
    filename = "src/overlays/actors/ovl_"
    if name != "player":
        filename += name
    else:
        filename += name + "_actor"
    filename += "/z_" + name.lower() + ".c"

    return filename

func_call_regexpr = re.compile(r'[a-zA-Z_\d]+\([^\)]*\)(\.[^\)]*\))?')
func_defs_regexpr = re.compile(r'[a-zA-Z_\d]+\([^\)]*\)(\.[^\)]*\))? {[^}]')
macrosRegexpr = re.compile(r'#define\s+([a-zA-Z_\d]+)(\([a-zA-Z_\d]+\))?\s+(.+?)(\n|//|/\*)')
enumsRegexpr = re.compile(r'enum\s+\{([^\}]+?)\}')

# Capture all function calls in the block, including arguments
def capture_calls(content):
    return [x.group() for x in re.finditer(func_call_regexpr, content)]

# Capture all function calls in the block, name only
def capture_call_names(content):
    return [x.group().split("(")[0] for x in re.finditer(func_call_regexpr, content)]

# Capture all function definitions in the block, including arguments
def capture_definitions(content):
    return [x.group() for x in re.finditer(func_defs_regexpr, content)]

# Capture all function definitions in the block, name only
def capture_definition_names(content):
    definitions = []
    for x in re.finditer(func_defs_regexpr, content):
        definitions.append(x.group().split("(")[0])
    return definitions

setupaction_regexpr = re.compile(r"_SetupAction+\([^\)]*\)(\.[^\)]*\))?;")

# Capture all calls to the setupaction function
def capture_setupaction_calls(content):
    return [x.group() for x in re.finditer(setupaction_regexpr, content)]

# Captures the function name of a setupaction call
def capture_setupaction_call_arg(content):
    transitionList = []
    for x in re.finditer(setupaction_regexpr, content):
        func = x.group().split(",")[1].strip().split(");")[0].strip()
        if func not in transitionList:
            transitionList.append(func)
    return transitionList

setaction_regexpr = re.compile(r"_SetAction+\([^\)]*\)(\.[^\)]*\))?;")

def capture_setaction_calls(content):
    return [x.group() for x in re.finditer(setaction_regexpr, content)]

def capture_setaction_call_arg(content):
    transitionList = []
    for x in re.finditer(setaction_regexpr, content):
        func = x.group().split(",")[2].strip().split(");")[0].strip()
        if func not in transitionList:
            transitionList.append(func)
    return transitionList

# Search for the function definition by supplied function name
def definition_by_name(content, name):
    for definition in capture_definitions(content):
        if name == definition.split("(")[0]:
            return definition.split("{")[0].strip()

# Obtain the entire code body of the function
def get_code_body(content, funcname) -> str:
    line_num = line_numbers_of_functions[index_of_func(funcname)]
    if line_num <= 0:
        return ""
    code = ""
    bracket_count = 1

    all_lines = content.splitlines(True)
    for raw_line in all_lines[line_num:len(all_lines)]:
        # Ignore commented stuff
        if "//" in raw_line:
            raw_line = raw_line[:raw_line.index("//")]

        bracket_count += raw_line.count("{")
        bracket_count -= raw_line.count("}")

        if bracket_count == 0:
            break
        else:
            code += raw_line
    return code

def getMacrosDefinitions(contents):
    macrosDefs = dict()
    for x in re.finditer(macrosRegexpr, contents):
        macroName = x.group(1).strip()
        macroParamsAux = x.group(2)
        macroBody = x.group(3).strip()

        macroParams = []
        if macroParamsAux is not None:
            for x in macroParamsAux.strip("(").strip(")").split(","):
                macroParams.append(x.strip())
        macrosDefs[macroName] = (macroParams, macroBody)
    return macrosDefs

def parseMacro(macros, macroExpr):
    macroCall = func_call_regexpr.match(macroExpr)
    if macroCall is not None:
        macroName, macroArgs = macroCall.group().split(")")[0].split("(")
        if macroName not in macros:
            print("Unknown macro: " + macroName)
            return None
        macroParams, macroBody = macros[macroName]
        argsList = [x.strip() for x in macroArgs.split(",")]

        macroBody = str(macroBody)
        for i, x in enumerate(macroParams):
            macroBody = macroBody.replace(x, argsList[i])
        return str(eval(macroBody))
    elif macroExpr in macros:
        macroParams, macroBody = macros[macroExpr]
        return str(eval(macroBody))
    return None

def getEnums(contents):
    enums = dict()
    for x in re.finditer(enumsRegexpr, contents):
        enumValue = 0
        for var in x.group(1).split(","):
            if "/*" in var and "*/" in var:
                start = var.index("/*")
                end = var.index("*/") + len("*/")
                var = var[:start] + var[end:]
            if "//" in var:
                var = var[:var.index("//")]
            var = var.strip()
            if len(var) == 0:
                continue

            enumName = var
            exprList = var.split("=")
            if len(exprList) > 1:
                enumName = exprList[0].strip()
                enumValue = int(exprList[1], 0)

            enums[enumName] = enumValue
            enumValue +=1

    return enums

def index_of_func(func_name):
    return func_names.index(func_name)

def action_var_setups_in_func(content, func_name, action_var):
    code_body = get_code_body(content, func_name)
    if action_var not in code_body:
        return None
    return [x.group() for x in re.finditer(r'(' + action_var + r' = (.)*)', code_body)]

def action_var_values_in_func(code_body, action_var, macros, enums):
    if action_var not in code_body:
        return list()

    regex = re.compile(r'(' + action_var + r' = (.)*)')
    transition = []
    for x in re.finditer(regex, code_body):
        index = x.group().split(" = ")[1].split(";")[0].strip()

        macroValue = parseMacro(macros, index)
        if macroValue is not None:
            index = macroValue
        elif index in enums:
            index = str(enums[index])

        if index not in transition:
            transition.append(index)
    return transition

def setup_line_numbers(content, func_names):
    global line_numbers_of_functions
    for line_no, line in enumerate(content.splitlines(True),1):
        for func_name in func_names:
            # Some functions have definitions on multiple lines, take the last
            if func_definitions[index_of_func(func_name)].split("\n")[-1] in line:
                line_numbers_of_functions.append(line_no)

def setup_func_definitions(content, func_names):
    global func_definitions
    for func_name in func_names:
        func_definitions.append(definition_by_name(content, func_name)+" {")


def addFunctionTransitionToGraph(dot, index: int, func_name: str, action_transition: str):
    fontColor = config.get("colors", "fontcolor")
    bubbleColor = config.get("colors", "bubbleColor")
    indexStr = str(index)
    try:
        funcIndex = str(index_of_func(action_transition))
    except ValueError:
        print(f"Warning: function '{action_transition}' called by '{func_name}' was not found. Skiping...", file=sys.stderr)
        return

    dot.node(indexStr, func_name, fontcolor=fontColor, color=bubbleColor)
    dot.node(funcIndex, action_transition, fontcolor=fontColor, color=bubbleColor)
    edgeColor = config.get("colors", "actionFunc")
    if func_name.endswith("_Init"):
        edgeColor = config.get("colors", "actionFuncInit")
    dot.edge(indexStr, funcIndex, color=edgeColor)

def addCallNamesToGraph(dot, func_names: list, index: int, code_body: str, removeList: list, setupAction=False, rawActorFunc=False):
    edgeColor = config.get("colors", "funcCall")
    fontColor = config.get("colors", "fontcolor")
    bubbleColor = config.get("colors", "bubbleColor")

    indexStr = str(index)
    seen = set()
    for call in capture_call_names(code_body):
        if call not in func_names:
            continue
        if call in seen:
            continue
        if call in removeList:
            continue

        if setupAction and ("_SetupAction" in call or "_SetAction" in call):
            continue
        seen.add(call)

        if rawActorFunc:
            dot.node(indexStr, func_names[index], fontcolor=fontColor, color=bubbleColor)

        calledFuncIndex = str(index_of_func(call))

        dot.node(calledFuncIndex, call, fontcolor=fontColor, color=bubbleColor)
        dot.edge(indexStr, calledFuncIndex, color=edgeColor)

def addCallbacksToGraph(dot, func_names: list, index: int, code_body: str, transitionList: list):
    edgeColor = config.get("colors", "callback")
    fontColor = config.get("colors", "fontcolor")
    bubbleColor = config.get("colors", "bubbleColor")

    indexStr = str(index)
    seen = set()
    for call_with_arguments in capture_calls(code_body):
        call_with_arguments = call_with_arguments.replace("\n", "").replace(" ", "")
        name, arguments = call_with_arguments.split("(", 1)
        argumentList = [x.strip() for x in arguments.split(",")]
        for callback in [x for x in func_names if x in argumentList]:
            if callback in transitionList:
                # already catched in another edge
                continue
            seen.add(callback)

            calledFuncIndex = str(index_of_func(callback))

            dot.node(calledFuncIndex, callback, fontcolor=fontColor, color=bubbleColor)
            dot.edge(indexStr, calledFuncIndex, color=edgeColor)

def loadConfigFile(selectedStyle):
    # For a list of colors, see https://www.graphviz.org/doc/info/colors.html
    # Hex colors works too!
    stylesFolder = os.path.join(script_dir, "graphovl_styles")
    configFilename = os.path.join(stylesFolder, "graphovl_config.ini")

    # Set defaults, just in case.
    config.add_section('colors')
    config.set('colors', 'background', 'white')
    config.set('colors', 'funcCall', 'blue')
    config.set('colors', 'actionFuncInit', 'green')
    config.set('colors', 'actionFunc', 'Black')
    config.set('colors', 'fontColor', 'Black')
    config.set('colors', 'bubbleColor', 'Black')
    config.set('colors', 'callback', 'blue')

    if os.path.exists(configFilename):
        config.read(configFilename)
    else:
        print("Warning! Config file not found.")

    style = config.get("config", "defaultStyle") + ".ini"
    if selectedStyle is not None:
        style = selectedStyle + ".ini"
    styleFilename = os.path.join(stylesFolder, style)

    if os.path.exists(styleFilename):
        config.read(styleFilename)
    else:
        print(f"Warning! Style {style} not found.")


def main():
    global func_names
    parser = argparse.ArgumentParser(description="Creates a graph of action functions (black and green arrows) and function calls (blue arrows) for a given overlay file")
    parser.add_argument("filename", help="Filename without the z_ or ovl_ prefix, e.x. Door_Ana")
    parser.add_argument("--loners", help="Include functions that are not called or call any other overlay function", action="store_true")
    parser.add_argument("-s", "--style", help="Use a color style defined in graphovl_styles folder. i.e. solarized")
    parser.add_argument("--format", help="Select output file format. Defaults to 'png'", default="png")
    parser.add_argument("-r", "--remove", help="A space-separated list of nodes to remove from the graph", nargs='+')
    args = parser.parse_args()

    removeList = []
    if args.remove is not None:
        removeList = args.remove

    loadConfigFile(args.style)
    fontColor = config.get("colors", "fontcolor")
    bubbleColor = config.get("colors", "bubbleColor")

    fname = args.filename
    dot = Digraph(comment = fname, format = args.format)
    dot.attr(bgcolor=config.get("colors", "background"))
    contents = ""

    with open(actor_src_path(fname), "r") as infile:
        contents = infile.read()

    func_names = capture_definition_names(contents)
    setup_func_definitions(contents, func_names)
    setup_line_numbers(contents, func_names)
    macros = getMacrosDefinitions(contents)
    enums = getEnums(contents)
    func_prefix = ""
    for index, func_name in enumerate(func_names):
        # Init is chosen because all actors are guaranteed to have an Init function.
        # This check is however required as not all functions may have been renamed yet.
        if func_name.endswith("_Init"): 
            func_prefix = func_name.split("_")[0]
            dot.node(str(index), func_name, fontcolor=fontColor, color=bubbleColor)
        elif (func_name.endswith("_Destroy") or func_name.endswith("_Update") or func_name.endswith("_Draw")):
            dot.node(str(index), func_name, fontcolor=fontColor, color=bubbleColor)

    action_func_type = func_prefix + "ActionFunc"
    match_obj = re.search(re.compile(action_func_type + r' (.+)\[\] = {'), contents)
    actionIdentifier = "this->actionFunc"

    setupAction = func_prefix + "_SetupAction" in func_names
    setAction = func_prefix + "_SetAction" in func_names
    arrayActorFunc = match_obj is not None
    rawActorFunc = actionIdentifier in contents

    if not setupAction and not setAction and not arrayActorFunc and not rawActorFunc:
        print("No actor action-based structure found")
        os._exit(1)

    action_functions = []
    action_var = ""
    if arrayActorFunc:
        action_func_array = re.search(action_func_type + r' (.+)\[\] = \{([^}]*?)\};', contents)
        if action_func_array is None:
            print("Invalid array-based actor.")
            print("Call action func array not found.")
            os._exit(1)
        actionFuncArrayElements = action_func_array.group(2).split(",")
        action_functions = [x.strip() for x in actionFuncArrayElements]

        action_func_array_name = match_obj.group(1).strip()
        actionVarMatch = re.search(action_func_array_name + r'\[(.*)\]\(', contents)
        if actionVarMatch is None:
            print("Invalid ActorFunc array-based actor.")
            print("Call to array function not found.")
            os._exit(1)
        action_var = actionVarMatch.group(1).strip()

    for index, func_name in enumerate(func_names):
        if func_name in removeList:
            continue

        indexStr = str(index)
        if args.loners:
            dot.node(indexStr, func_name, fontcolor=fontColor, color=bubbleColor)
        code_body = get_code_body(contents, func_name)

        transitionList = []
        if setupAction:
            """
            Create all edges for SetupAction-based actors
            """
            transitionList = capture_setupaction_call_arg(code_body)
        elif setAction:
            transitionList = capture_setaction_call_arg(code_body)
        elif arrayActorFunc:
            """
            Create all edges for ActorFunc array-based actors
            """
            transitionIndexes = action_var_values_in_func(code_body, action_var, macros, enums)
            transitionList = [action_functions[int(index, 0)] for index in transitionIndexes]
        elif rawActorFunc:
            """
            Create all edges for raw ActorFunc-based actors
            """
            transitionList = action_var_values_in_func(code_body, actionIdentifier, macros, enums)

        # Remove functions calls
        transitionList = [x for x in transitionList if x not in removeList]

        for action_transition in transitionList:
            addFunctionTransitionToGraph(dot, index, func_name, action_transition)

        addCallNamesToGraph(dot, func_names, index, code_body, removeList, setupAction, rawActorFunc)

        addCallbacksToGraph(dot, func_names, index, code_body, transitionList)

    # print(dot.source)
    outname = f"graphs/{fname}.gv"
    dot.render(outname)
    print(f"Written to {outname}.{args.format}")

if __name__ == "__main__":
    main()
