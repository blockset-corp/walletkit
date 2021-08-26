#!/usr/bin/env bash
#
#  make_blockchain.sh 
#  walletkit 
#
#  Created by Bryan Goring on 07/07/21.
#  Copyright © 2019 Breadwinner AG. All rights reserved.
#
#  See the LICENSE file at the project root for license information.
#  See the CONTRIBUTORS file at the project root for a list of contributors.

# This script instantiates a new empty, complete blockchain & handler
# code stub into the appropriate WalletKitCore src folder. Either
# handlers or blockchain (or both) can be created. The script also 
# updates global header files that are aware of the various blockchains,
# and modifies makefiles etc, so that it can be built. As such, it makes
# certain assumptions about locations of files and folders within WalletKitCore/src
# and is also dependent on the presence of, and organization of certain
# input template files.
#
# As a test the script can be run with a different output folder (-o) than
# the actual location of 'WalletKitCore' (which is otherwise defaulted) to
# see the effect of the new handler and blockchain creation effect.

# Global: Identifiers should be simple character mnemonics made up of
#         alphanumerics, prefereably upper and lower case letters, and
#         should not contain whitespace or special characters.
#
#   gen_user:           User name for decorating copyright notices
#   bc_symbol:          A blockchain short identifier (e.g. XTZ) 
#   bc_name:            A short descriptive name for the blockchain (e.g. Tezos)
#   core_packages       The number of core packages
#   swift_packages      The number of swift packages
#   packages:           Default packages to be produced for the new blockchain; handlers & blockchain
#                       implementation 
#   root:               Output folder for all generation including core, Swift, Java
#   core_pkg_paths      Initial paths within src folders for output blockchain code
#   swift_pkg_paths     Initial paths within src folders for output swift code
#   pkg_paths           Working directories based on customized core_pkg_paths 
#   templates           Location of code templates. The organization of folders and files
#                       within templates is considered fixed
#   wk_config_templ     A template for definitions belonging to the main configuration file
#   build_bc_templ      A template for creating blockchain impl specific build sections 
#                       of the CMakeLists.txt file
#   build_hndlr_templ   A template for creating handler specific build sections of the
#                       CMakeLists.txt file
#   include_testnet     Indicates to create a template that includes testnet

# Exit on error
set -e

gen_user=""
bc_symbol=""
bc_name=""

# Number of and activity of all packages
core_packages=3
swift_packages=1
packages=(1 1 1 1 1)
package_names=("handlers" "blockchain" "test" "Swift" "Java")
root="$(dirname $0)/.."
output_path=()
core_pkg_paths=("src/walletkit/handlers" "src" "WalletKitCoreTests/test")
swift_pkg_paths=("WalletKit")
pkg_paths=()
templates="${root}/templates"
wk_config_templ="WkConfig.h.tmpl"
build_bc_templ="make/cmakelists.bc.tmpl"
build_hndlr_templ="make/cmakelists.hndlr.tmpl"
build_test_templ="make/cmakelists.test.tmpl"
include_testnet=0
verbose=0

# Types of packages
HANDLER_PKG=0
BLOCKCHAIN_PKG=1
TESTS_PKG=2
SWIFT_PKG=3
JAVA_PKG=4

# Locations of output
WKCORE_OUTPUT=0
SWIFT_OUTPUT=1
JAVA_OUTPUT=2

# Simple echo screening desired verbosity
echov() {
    if [ $verbose -gt 0 ]; then
        echo "$1"
    fi
}

# Program usage
# @param error_msg Optional error message for usage
usage() {

    error_msg="$1"
    if [ "$error_msg" != "" ]; then
        echo "Err: $error_msg"
    fi

    echo "Usage: $0 [options] <blockchain_symbol> <blockchain_name>"
    echo
    echo "       where:"
    echo "          <blockchain_symbol> is the main shortened mnemonic associated"
    echo "                              with the blockchain (e.g. 'XTZ')"
    echo
    echo "          <blockchain_name> specifies the blockchain descriptive name (e.g. 'Tezos')"
    echo
    echo "       options:"
    echo
    echo "          -g: Specifies a code package index to include, and controls which type of"
    echo "             output will be generated:"
    echo "                0: handlers package"
    echo "                1: blockchain implementation package"
    echo "                2: tests"
    echo "                3: Swift"
    echo "                3: Java"
    echo "             By default all packages are generated"
    echo
    echo "          -o: Specifies the output folder for generated code"
    echo "          -i: Path to templates"
    echo "          -v: Verbose output"
    echo "          -t: Creates header files supporting testnet (default mainnet)"
    echo "          -u: Username for decorating copyright notices (otherwise shell defaults)"
    echo "          -h: help"
}

# Gets all program options and populates global variables.
#
# Requires at least two arguments containing the blockchain symbol
# and blockname name.
#
# @param bc_symbol The blockchain mnemonic 
# @param bc_name The name of the new blockchain
get_opts() {

    custom_pkgs=(0 0 0 0 0)
    gens=0
    parm_num=0

    while getopts ":g:o:i:tvhu:" o; do
        
        case "$o" in
        g)
            if [ $OPTARG -lt ${#packages[@]} ]; then
                custom_pkgs[$OPTARG]=1
                gens=$(($gens + 1))
            else
                usage "Invalid package index: $OPTARG"
                exit 1
            fi
            parm_num=$(($parm_num+1))
            ;;

        o)
            root=$OPTARG
            parm_num=$(($parm_num+1))
            ;;

        u)
            gen_user="$OPTARG"
            parm_num=$(($parm_num+1))
            ;;

        i)
            templates=$OPTARG
            parm_num=$(($parm_num+1))
            ;;

        t)
            include_testnet=1
            ;;

        v)
            verbose=1
            ;;

        h) 
            usage
            exit 0
            ;;

        *)
            usage
            exit 1
            ;;

        esac

        parm_num=$(($parm_num+1))
    done
    
    # Requires an additional 2 parameters for
    # non-optional blockchain_symbol and blockchain_name
    if [ ${#@} -lt $(($parm_num + 2)) ]; then
        usage "Provide blockchain (s)ymbol and (n)ame"
        exit
    fi
   
    # Non-default package generation 
    if [ $gens -gt 0 ]; then
        packages=(${custom_pkgs[*]})
    fi

    bc_symbol=${@: -2:1}
    bc_name=${@: -1}
}

# Checks if any of the indicated packages are enabled
# Sets the pkg_enabled_result to indicate the result
# status.
pkg_enabled() {
    indexes=("$@")
    for (( i=0; i<${#indexes[@]}; i++ )); do
        if [ ${indexes[$i]} -lt ${#packages[@]} ] &&
           [ ${packages[${indexes[$i]}]} -ne 0 ]; then
            pkg_enabled_result=1 
            return
        fi 

    done

    pkg_enabled_result=0 
}

# Creates output folders for created packages
# This function also updates the pkg_paths for each package type (handler, blockchain etc)
# with the specific folder names being created for this blockchain
#
# param bc_folder: The blockchain specific folder name
check_create_folders() {
    
    pkg_folders=("$@") 

    # Create required forders 
    out_path=${output_path[$WKCORE_OUTPUT]}

    for (( i=0; i<$core_packages; i++)); do

        if [ ${packages[$i]} -ne 0 ]; then
            # Remember the final new output package path, per package type
            pkg_paths[$i]=$out_path/${core_pkg_paths[$i]}/${pkg_folders[$i]}
            echov "  Create package path ${pkg_paths[$i]}"
            mkdir -p ${pkg_paths[$i]}
        fi
    done
}

# Copy all required templates for all requested packages, making
# naming substitution based on desired blockchain names as we go
#
# @param symbols The replacement symbols for template files
copy_templates() {

    symbols=("$@") 

    echov
    for (( i=0; i<$core_packages; i++)); do
        if [ ${packages[$i]} -ne 0 ]; then

            template_src_folder=$templates/${package_names[$i]}
            templates_files=`ls $template_src_folder`
            
            for template_file in $templates_files; do
                cpFrom=$template_src_folder/$template_file
                cpTo=${pkg_paths[$i]}/${template_file/${symbols[$i*2]}/${symbols[$i*2+1]}}
                cp $cpFrom $cpTo
                echov "    Creating file $cpTo"
            done
        fi
    done
    echov
}

# Personalizes the templated package with new blockchain names
#
# @param pkg_path The path to where fresh & unmodified blockchain (from templates) 
#                 files are located
# @param symbols All the replacement symbols and their blockchain specific
#                replacement values, as an array
personalize_package() {
    pkg_path=$1
    symbols=(${@:2})

    pkg_files=`ls $pkg_path`
    for pkg_file in $pkg_files; do 
       
        for (( replace=0; replace<${#symbols[@]}; replace+=2 )); do 
            # Note: due to differences with in-place editing of files with sed
            # on OSX vs GNU, the following may not work on GNU
            sed -i '' -e "1,\$s/${symbols[replace]}/${symbols[replace+1]//\\s/ }/g" $pkg_path/$pkg_file
        done 

    done
}

# Updates the new or existing WKConfig.h file which is the header files
# knowledgable of all handlers
#
# @param wkconfig The input WKConfig.h file location
# @param symbol The blockchain symbol
# @param symbols Replacement strings and symbols for the template that
#                is modified and inserted into WKConfig.h file 
update_wkconfig() {
    wkconfig=$1 
    symbol=$2
    symbols=(${@:3}) 
   
    bare=`cat $templates/$wk_config_templ`

    # Create new personalized wkconfig section    
    for (( i=0; i<${#symbols[@]}; i+=2 )); do
        bare=${bare//${symbols[i]}/${symbols[i+1]}}
    done
   
    # Following is requred to make the replacement string fit
    # for sed: escape comments, escape actual newlines from the
    # input template file with escaped newlines in the output
    bare=${bare//\//\\/} 
    bare=${bare//$'\n'/\\n}

    # Defintions for new handler go in place of // __CONFIG__ 
    sed -i '' -e "s/\/\/[ ]__CONFIG__/$bare/" $wkconfig 

    # Control of testnet availability go in place of // __CONTROL__
    testnet_define="#define HAS_${symbol}_TESTNET $include_testnet\\n\\/\\/ __CONTROL__"
    sed -i '' -e "s/\/\/[ ]__CONTROL__/$testnet_define/" $wkconfig
}

# Update the cmakelists file to build new package
#
# @param mfile The path to cmakelists
# @param tmpl The pkg section declaring template file to use
# @param replace The cmakelists pkg section string to replace
# @param num_symbols The number of elements in the symbols array
# @param symbols The symbols and replacements to be done on the bare template
# @param fills Fills for the template; the replacement token and the location 
#              of build files (.c, .h) that should be fed into it 
update_cmakelists() {

    parent_path=$1
    mfile=$2
    tmpl=$3
    repl=$4
    num_symbols=$5
    symbols=(${@:6:$num_symbols})
    fills=(${@:$((6 + $num_symbols))})

    # Start with bare template and replace symbols
    cmake=`cat $templates/$tmpl`
    for (( i=0; i<${#symbols[@]}; i+=2 )); do
        cmake=${cmake//${symbols[i]}/${symbols[i+1]}}
    done

    for (( i=0; i<${#fills[@]}; i+=2 )); do
        
        # Create the list of buildable files
        # suitable for core cmakelists 
        files=(`ls ${fills[i+1]}`)
        compilable=""
        for (( j=0; j<${#files[@]}; j++ )); do
            pth=${fills[i+1]}
            
            pth=${pth/$parent_path/}
            compilable="$compilable        \${PROJECT_SOURCE_DIR}${pth//\//\\/}\/${files[j]}\n"
        done
        cmake=${cmake//${fills[i]}/$compilable}
        cmake=${cmake//$'\n'/\\n}
        
        # Substitute the template section into
        # the makefile
        sed -i '' -e "s/$repl/$cmake/" $mfile
    done
    
}

# Checks if there is a cmakelists in the target output
# folder and if not, creates one
function check_create_cmakelists() {
    # Update build files for new handler
    cmakelists_file=${output_path[$WKCORE_OUTPUT]}/CMakeLists.txt
    if [ ! -f $cmakelists_file ]; then
        echov "  Creating new CMakeLists.txt file"
        touch $cmakelists_file
        echo $'#' __NEW_BLOCKCHAIN_IMPL__$'\n'$'\n' >> $cmakelists_file
        echo $'#' __NEW_HANDLER__$'\n'$'\n' >> $cmakelists_file
        echo $'#' __NEW_BLOCKCHAIN_TEST__$'\n'$'\n' >> $cmakelists_file
    fi
}

# Checks if there is a WKConfig.h in the target output
# folder and if not, creates one
function check_create_wkconfig() {
    wk_config_file=${output_path[$WKCORE_OUTPUT]}/src/walletkit/WKConfig.h
    if [ ! -f $wk_config_file ]; then
        echov "  Creating new WKConfig.h file"
        touch $wk_config_file
        echo // __CONTROL__$'\n'$'\n'// __CONFIG__$'\n'$'\n' >> $wk_config_file
    fi
}

# Checks if there is a WKBase.h in the target output
# folder and if not, creates one
function check_create_wkbase() {
    wk_base_file=${output_path[$WKCORE_OUTPUT]}/include/WKBase.h
    if [ ! -f $wk_base_file ]; then
        echov "  Creating new WKBase.h file"
        mkdir -p ${output_path[$WKCORE_OUTPUT]}/include
        touch $wk_base_file
        
        echo $'#'ifndef WKBase.h$'\n'$'#'define WKBase.h$'\n' >> $wk_base_file
        echo typedef enum {$'\n''    '/* WK_NETWORK_TYPE___SYMBOL__' '*/$'\n'} WKNetworkType$';'$'\n' >> $wk_base_file
        echo $'#'define WK_NETWORK_TYPE_LAST'        'WK_NETWORK_TYPE_XLM$'\n' >> $wk_base_file
        echo "/* #define WK_NETWORK_CURRENCY___SYMBOL__    \"__symbol__\" */"$'\n' >> $wk_base_file
        echo $'#'endif$'\n' >> $wk_base_file
    fi
}

# Checks if there is no swift wrapper of tests and 
# creates on if need be
function check_create_walletkitcoretests() {

    wkcoretests_base_file=${output_path[$WKCORE_OUTPUT]}/WalletKitCoreTests/WalletKitCoreTests.swift
    wkcoretests_include=${output_path[$WKCORE_OUTPUT]}/WalletKitCoreTests/test/include/test.h

    if [ ! -f $wkcoretests_base_file ]; then
        echov "  Creating new WalletKitCoreTests.swift file"
        mkdir -p ${output_path[$WKCORE_OUTPUT]}/WalletKitCoreTests/test
        touch $wkcoretests_base_file

        echo '    '//' '__NEW_BLOCKCHAIN_TEST_IMPL__$'\n' >> $wkcoretests_base_file
        echo '    'static var allTests = [$'\n''        '//' '__NEW_BLOCKCHAIN_TEST__$'\n''    '] >> $wkcoretests_base_file
    fi
    if [ ! -f $wkcoretests_include ]; then
        echov "  Creating new tests header file"
        mkdir -p ${output_path[$WKCORE_OUTPUT]}/WalletKitCoreTests/test/include
        touch $wkcoretests_include

        echo //' '__NEW_BLOCKCHAIN_TEST_DEFN__$'\n' >> $wkcoretests_include
    fi
}

# Update the swift tests to include a new bare test for the 
# particular blockchain being added
#
# @param name The Name of the new blockchain
function update_walletkitcoretests() {

    name=$1
    wkcoretests_base_file=${output_path[$WKCORE_OUTPUT]}/WalletKitCoreTests/WalletKitCoreTests.swift
    wkcoretests_include=${output_path[$WKCORE_OUTPUT]}/WalletKitCoreTests/test/include/test.h

    all_tests_inclusion="\/\/ ${name}\n        (\"test${name}\", test${name}),\n"
    all_tests_inclusion="$all_tests_inclusion        \/\/ __NEW_BLOCKCHAIN_TEST__"
    test_impl="\/\/ MARK - ${name}\n    func test${name} () {\n"
    test_impl="$test_impl        run${name}Test()\n    }\n\n    \/\/ __NEW_BLOCKCHAIN_TEST_IMPL__"
    test_defn="\/\/ ${name}\nextern void\nrun${name}Test (void);\n\n\/\/ __NEW_BLOCKCHAIN_TEST_DEFN__"
    sed -i '' -e "1,\$s/\/\/[ ]__NEW_BLOCKCHAIN_TEST__/$all_tests_inclusion/" $wkcoretests_base_file
    sed -i '' -e "1,\$s/\/\/[ ]__NEW_BLOCKCHAIN_TEST_IMPL__/$test_impl/" $wkcoretests_base_file
    sed -i '' -e "1,\$s/\/\/[ ]__NEW_BLOCKCHAIN_TEST_DEFN__/$test_defn/" $wkcoretests_include
}

# Checks for the WKAccount.c file and if not existant, create a test file
function check_create_wkaccount() {
    wk_account_file=${output_path[$WKCORE_OUTPUT]}/src/walletkit/WKAccount.c
    if [ ! -f $wk_account_file ]; then

        echov "  Creating new WKAccount.c file"
        mkdir -p ${output_path[$WKCORE_OUTPUT]}/src/walletkit
        touch $wk_account_file
       
        echo // Version 6: The prior version >> $wk_account_file
        echo "#define ACCOUNT_SERIALIZE_DEFAULT_VERSION  6"  >> $wk_account_file 
    fi
}

# Create if necessary WKAccount.c and then update with the required
# account serialization version which is bumped with each new blockchain
#
# @param ucsymbol The UpperCase blockchain mnemonic
function update_wkaccount() {
    
    ucsymbol=$1

    wk_account_file=${output_path[$WKCORE_OUTPUT]}/src/walletkit/WKAccount.c
    check_create_wkaccount

    # WKNetworkType private static decl for new VALUE requires picking up
    # the last emitted 'next value' which is part of the commented section
    acct_ser_vers_repl="#define ACCOUNT_SERIALIZE_DEFAULT_VERSION\\s+[0-9]{1,}"
    last_vers_value=`grep -Eo "$acct_ser_vers_repl" $wk_account_file | grep -Eo '[0-9]{1,}'`
    next_vers_value=$((last_vers_value + 1))
    next_vers_comment="\/\/ Version ${next_vers_value}: V${last_vers_value} + ${ucsymbol}"
    next_vers="#define ACCOUNT_SERIALIZE_DEFAULT_VERSION ${next_vers_value}"
    sed -i '' -e "1,\$s/#define ACCOUNT_SERIALIZE_DEFAULT_VERSION[[:space:]]*${last_vers_value}/$next_vers_comment\n$next_vers/" $wk_account_file 
}

# Checks for the WKNetwork.c file and if not existant, create a test file
function check_create_wknetwork() {
    wknetwork_c_file=${output_path[$WKCORE_OUTPUT]}/src/walletkit/WKNetwork.c
    if [ ! -f $wknetwork_c_file ]; then

        echov "  Creating new WKNetwork.c file"
        mkdir -p ${output_path[$WKCORE_OUTPUT]}/src/walletkit
        touch $wknetwork_c_file
       
        # wkNetworkTypeGetCurrencyCode
        echo "extern const char *" >> $wknetwork_c_file
        echo "wkNetworkTypeGetCurrencyCode (WKNetworkType type) {" >> $wknetwork_c_file
        echo "    static const char *currencies [NUMBER_OF_NETWORK_TYPES] = {" >> $wknetwork_c_file
        echo "        WK_NETWORK_CURRENCY_XLM," >> $wknetwork_c_file
        echo "        /* WK_NETWORK_CURRENCY___SYMBOL__, */" >> $wknetwork_c_file
        echo "     };" >> $wknetwork_c_file
        echo "     return currencies[type];" >> $wknetwork_c_file
        echo "}" >> $wknetwork_c_file

        # wkNetworkTypeIsBitcoinBased
        echo "private_extern bool" >> $wknetwork_c_file
        echo "wkNetworkTypeIsBitcoinBased (WKNetworkType type) {" >> $wknetwork_c_file
        echo "    static const char isBitcoinBased [NUMBER_OF_NETWORK_TYPES] = {" >> $wknetwork_c_file
        echo "        false,      // XLM" >> $wknetwork_c_file
        echo "     /* false,      // __SYMBOL__ */" >> $wknetwork_c_file
        echo "    };" >> $wknetwork_c_file
        echo "    return isBitcoinBased[type];" >> $wknetwork_c_file
        echo "}" >> $wknetwork_c_file
    fi
}

# Create if necessary WKNetwork.c and then update with the required
# account serialization version which is bumped with each new blockchain
#
# @param ucsymbol The UpperCase blockchain mnemonic
function update_wknetwork() {
    
    ucsymbol=$1

    wknetwork_c_file=${output_path[$WKCORE_OUTPUT]}/src/walletkit/WKNetwork.c
    check_create_wknetwork

    # Update wkNetworkTypeGetCurrencyCode 
    cur_def_repl="/* WK_NETWORK_CURRENCY___SYMBOL__, */"
    cur_def_val=${cur_def_repl//__SYMBOL__/$ucsymbol}
    cur_def_repl_escpd=${cur_def_repl//\//\\/}
    cur_def_repl_escpd=${cur_def_repl_escpd//\*/\\*}
    cur_def_val=${cur_def_val//\/\*/}
    cur_def_val=${cur_def_val//\*\//}
    cur_def_val=${cur_def_val// /}
    sed -i '' -e "1,\$s/$cur_def_repl_escpd/$cur_def_val\n        $cur_def_repl_escpd/" $wknetwork_c_file 

    # Update wkNetworkTypeIsBitcoinBased
    bc_based_repl="/* false,      // __SYMBOL__ */"
    bc_based_val="  ${bc_based_repl//__SYMBOL__/$ucsymbol}"
    bc_based_repl_escpd=${bc_based_repl//\//\\/}
    bc_based_repl_escpd=${bc_based_repl_escpd//\*/\\*}
    bc_based_val=${bc_based_val//\/\*/}
    bc_based_val=${bc_based_val//\*\//}
    bc_based_val=${bc_based_val//\//\\/}
    sed -i '' -e "1,\$s/$bc_based_repl_escpd/$bc_based_val\n     $bc_based_repl_escpd/" $wknetwork_c_file 
}

# Updates the WKBase.h header file with required definitions
#
# @param ucsymbol The UpperCase blockchain mnemonic
# @param lcsymbol The lowercase blockchain mnemonic
function update_wkbase() {

    ucsymbol=$1
    lcsymbol=$2

    wk_base_file=${output_path[$WKCORE_OUTPUT]}/include/WKBase.h

    # Create if necessary    
    check_create_wkbase

    net_new_type="WK_NETWORK_TYPE___SYMBOL__"
    net_new_val=${net_new_type/__SYMBOL__/$ucsymbol}
    net_type_repl="/* $net_new_type */" 
    net_currency_repl="/* #define WK_NETWORK_CURRENCY___SYMBOL__    \"__symbol__\" */"
    net_type=${net_type_repl/__SYMBOL__/$ucsymbol}
    net_currency=${net_currency_repl/__SYMBOL__/$ucsymbol}
    net_currency=${net_currency/__symbol__/$lcsymbol}

    # Escape replacement expressions and values suitable for sed
    net_type_repl=${net_type_repl//\//\\/}
    net_type_repl=${net_type_repl//\*/\\*}
    net_currency_repl=${net_currency_repl//\//\\/}
    net_currency_repl=${net_currency_repl//\*/\\*}

    # Remove comments & spaces from replacement value
    net_type=${net_type//\/\*/}
    net_type=${net_type//\*\//}
    net_type=${net_type// /}
    net_currency=${net_currency//\/\*/}
    net_currency=${net_currency//\*\//}
    net_currency=${net_currency:1:${#net_currency}-1}
    
    # Replacement of actual values and restore replacement string thereafter

    # WKNetworkType enumeration
    sed -i '' -e "1,\$s/$net_type_repl/$net_type,\n    $net_type_repl/" $wk_base_file

    # Introduce a new WK_NETWORK_CURRENCY... 
    sed -i '' -e "1,\$s/$net_currency_repl/$net_currency\n$net_currency_repl/" $wk_base_file 

    # Update the 'last' type enumeration element for defining total number
    # of network types
    sed -i '' -e "1,\$s/#define WK_NETWORK_TYPE_LAST\([ ]*\)WK_NETWORK_TYPE_\(.*\)/#define WK_NETWORK_TYPE_LAST\1$net_new_val/" $wk_base_file
}

# Checks of the WKNetwork.swift file exists, and if not creates a test
# file with some basic content for generation on
function check_create_swift() {
    
    wknetwork_swift=${output_path[$SWIFT_OUTPUT]}/WalletKit/WKNetwork.swift

    if [ ! -f $wknetwork_swift ]; then
        echov "  Creating new WKNetwork.swift file"
        mkdir -p ${output_path[$SWIFT_OUTPUT]}/WalletKit
        touch $wknetwork_swift

        # Generate something base to modify
        echo "public enum NetworkType: CustomStringConvertable {" >> $wknetwork_swift
        echo "    /* case __symbol__ */" >> $wknetwork_swift
        echo "    internal init (core: WKNetworkType) {" >> $wknetwork_swift
        echo "        switch core {" >> $wknetwork_swift
        echo "            /* case WK_NETWORK_TYPE___SYMBOL__: self = .__symbol__ */" >> $wknetwork_swift
        echo "            default: preconditionFailure()" >> $wknetwork_swift
        echo "        }" >> $wknetwork_swift
        echo "    }" >> $wknetwork_swift         
        echo "    internal var core: WKNetworkType {" >> $wknetwork_swift
        echo "        switch self {" >> $wknetwork_swift
        echo "        /* case .__symbol__: return WK_NETWORK_TYPE___SYMBOL__ */" >> $wknetwork_swift
        echo "       }" >> $wknetwork_swift
        echo "   }" >> $wknetwork_swift
        echo "}" >> $wknetwork_swift
    fi
}

# Update the swift application impacted by WKNetworkType definition changes
#
# @param symbol The blockchain mnemonic
# @param symbol_lc The lower case blockchain mnemonic
update_swift() {

    symbol=$1
    symbol_lc=$2
    wknetwork_swift=${output_path[$SWIFT_OUTPUT]}/WalletKit/WKNetwork.swift

    repls=("/* case __symbol__ */"
           "/* case WK_NETWORK_TYPE___SYMBOL__: self = .__symbol__ */"
           "/* case .__symbol__: return WK_NETWORK_TYPE___SYMBOL__ */")
    vals=("    " 
          "        " 
          "        ")
    for (( i=0; i<${#repls[@]}; i++ )); do
        repl=${repls[$i]}
        val=${repl//__symbol__/$symbol_lc}
        val=${val//__SYMBOL__/$symbol}
        val=${val//\/\* /}
        val=${val// \*\//}
        val=$val'\n'${vals[$i]}$repl
        vals[$i]="$val"
    done

    # Now actually do the replacement
    for (( i=0; i<${#repls[@]}; i++ )); do
        repl=${repls[$i]}
        val=${vals[$i]}

        repl=${repl//\//\\/}
        repl=${repl//\*/\\*}
        val=${val//\//\\/}
        val=${val//\*/\\*}

        sed -i '' -e "1,\$s/$repl/$val/" $wknetwork_swift
    done
}     

# Creates test area for Java code update
check_create_java() {
    
    wknetwork_java=${output_path[$JAVA_OUTPUT]}/WalletKit/src/main/java/com/blockset/walletkit/NetworkType.java
    brdutil_java=${output_path[$JAVA_OUTPUT]}/WalletKitBRD/src/main/java/com/blockset/walletkit/brd/Utilities.java
    wknetworktype_java=${output_path[$JAVA_OUTPUT]}/WalletKitNative/src/main/java/com/blockset/walletkit/nativex/WKNetworkType.java

    if [ ! -f $wknetwork_java ]; then
        echov "  Creating new Java NetworkType file"
        mkdir -p ${output_path[$JAVA_OUTPUT]}/WalletKit/src/main/java/com/blockset/walletkit
        touch $wknetwork_java

        # Generate something base to modify
        echo "public enum NetworkType {" >> $wknetwork_java
        echo "    /* __SYMBOL__ */" >> $wknetwork_java
        echo "}" >> $wknetwork_java
    fi

    if [ ! -f $brdutil_java ]; then
        echov "  Creating new BRD Utilities file"
        mkdir -p ${output_path[$JAVA_OUTPUT]}/WalletKitBRD/src/main/java/com/blockset/walletkit/brd
        touch $brdutil_java

        echo "    static WKNetworkType networkTypeToCrypto(NetworkType type) {" >> $brdutil_java
        echo "        switch (type) {" >> $brdutil_java
        echo "            /* case __SYMBOL__: return WKNetworkType.__SYMBOL__; */" >> $brdutil_java
        echo "            default: throw new IllegalArgumentException(\"Unsupported type\");" >> $brdutil_java
        echo "        }" >> $brdutil_java
        echo "    }" >> $brdutil_java
        echo >> $brdutil_java 
        echo "    static NetworkType networkTypeFromCrypto(WKNetworkType type) {" >> $brdutil_java
        echo "        switch (type) {" >> $brdutil_java
        echo "            /* case __SYMBOL__: return NetworkType.__SYMBOL__; */" >> $brdutil_java
        echo "            default: throw new IllegalArgumentException(\"Unsupported type\");" >> $brdutil_java
        echo "        }" >> $brdutil_java
        echo "    }" >> $brdutil_java
        echo  >> $brdutil_java
    fi

    if [ ! -f $wknetworktype_java ]; then
        echov "  Creating new BRD WKNetworkType file"
        mkdir -p ${output_path[$JAVA_OUTPUT]}/WalletKitNative/src/main/java/com/blockset/walletkit/nativex
        touch $wknetworktype_java

        echo "public enum WKNetworkType {" >> $wknetworktype_java
        echo "    XLM {" >> $wknetworktype_java
        echo "        @Override" >> $wknetworktype_java
        echo "        public int toCore() {" >> $wknetworktype_java
        echo "            return XLM_VALUE;" >> $wknetworktype_java
        echo "        }" >> $wknetworktype_java
        echo "    }/* New __SYMBOL__ toCore() */;" >> $wknetworktype_java
        echo >> $wknetworktype_java
        echo "    /* private static final int __SYMBOL___VALUE = 10 */" >> $wknetworktype_java
        echo >> $wknetworktype_java
        echo "    public static WKNetworkType fromCore(int nativeValue) {" >> $wknetworktype_java
        echo "        switch (nativeValue) {" >> $wknetworktype_java
        echo "            /* case __SYMBOL___VALUE: return __SYMBOL__; */" >> $wknetworktype_java
        echo "            default: throw new IllegalArgumentException("Invalid core value");" >> $wknetworktype_java
        echo "        }" >> $wknetworktype_java
        echo "    }" >> $wknetworktype_java
        echo "}" >> $wknetworktype_java
        echo >> $wknetworktype_java
    fi
}

# Update Java NetworkType.java include new blockchain mnemonics
#
# @param symbol The blockchain mnemonic
update_networktype_java() {
    symbol=$1

    wknetwork_java=${output_path[$JAVA_OUTPUT]}/WalletKit/src/main/java/com/blockset/walletkit/NetworkType.java
    
    # NetworkType.java: NetworkType enumeration 
    nt_enum_repl="/* __SYMBOL__ */"
    nt_enum_val=${nt_enum_repl//__SYMBOL__/$symbol}
    nt_enum_repl_escpd=${nt_enum_repl//\//\\/}
    nt_enum_repl_escpd=${nt_enum_repl_escpd//\*/\\*}
    nt_enum_val=${nt_enum_val//\/\*/}
    nt_enum_val=${nt_enum_val//\*\//}
    nt_enum_val=${nt_enum_val// /}

    sed -i '' -e "1,\$s/$nt_enum_repl_escpd/$nt_enum_val,\n    $nt_enum_repl_escpd/" $wknetwork_java 
}

# Update BRD Utilities.java to include new blockchain mnemonics
#
# @param symbol The blockchain mnemonic
update_brd_utilities_java() {
    symbol=$1

    brdutil_java=${output_path[$JAVA_OUTPUT]}/WalletKitBRD/src/main/java/com/blockset/walletkit/brd/Utilities.java
    
    # Utilities.java: networkTypeToCrypto (tc), networkTypeFromCrypto (fc) function updates
    tc_repl="/* case __SYMBOL__: return WKNetworkType.__SYMBOL__; */"
    fc_repl="/* case __SYMBOL__: return NetworkType.__SYMBOL__; */"
    tc_val=${tc_repl//__SYMBOL__/$symbol}
    fc_val=${fc_repl//__SYMBOL__/$symbol}
    tc_repl_escpd=${tc_repl//\//\\/}
    tc_repl_escpd=${tc_repl_escpd//\*/\\*}
    fc_repl_escpd=${fc_repl//\//\\/}
    fc_repl_escpd=${fc_repl_escpd//\*/\\*}
    tc_val=${tc_val//\/\*/}
    tc_val=${tc_val//\*\//}
    tc_val=${tc_val:1:${#tc_val}}
    fc_val=${fc_val//\/\*/}
    fc_val=${fc_val//\*\//}
    fc_val=${fc_val:1:${#fc_val}}
   
    sed -i '' -e "1,\$s/$tc_repl_escpd/$tc_val\n            $tc_repl_escpd/" $brdutil_java 
    sed -i '' -e "1,\$s/$fc_repl_escpd/$fc_val\n            $fc_repl_escpd/" $brdutil_java 
}

update_native_wknetworktype_java() {
    symbol=$1

    wknetworktype_java=${output_path[$JAVA_OUTPUT]}/WalletKitNative/src/main/java/com/blockset/walletkit/nativex/WKNetworkType.java
    
    # toCore() method: replace a single line placeholder in WKNetworkType.java with a multiline method defn
    symbol_to_core=",\n\n    $symbol {\n"
    symbol_to_core="$symbol_to_core        @Override\n"
    symbol_to_core="$symbol_to_core        public int toCore() {\n"
    symbol_to_core="$symbol_to_core            return ${symbol}_VALUE;\n"
    symbol_to_core="$symbol_to_core        }\n"
    symbol_to_core="$symbol_to_core    }\/\* New __SYMBOL__ toCore() \*\/;\n"

    new_tocore_repl="/* New __SYMBOL__ toCore() */;"
    new_tocore_repl_escpd=${new_tocore_repl//\//\\/}
    new_tocore_repl_escpd=${new_tocore_repl_escpd//\*/\\*}

    sed -i '' -e "1,\$s/$new_tocore_repl_escpd/$symbol_to_core/" $wknetworktype_java 

    # WKNetworkType fromCore switch
    new_case_repl="/* case __SYMBOL___VALUE: return __SYMBOL__; */"
    new_case_val=${new_case_repl//__SYMBOL__/$symbol}
    new_case_repl_escpd=${new_case_repl//\//\\/}
    new_case_repl_escpd=${new_case_repl_escpd//\*/\\*}
    new_case_val=${new_case_val//\/\*/}
    new_case_val=${new_case_val//\*\//}
    new_case_val=${new_case_val:1:${#new_case_val}}

    sed -i '' -e "1,\$s/$new_case_repl_escpd/$new_case_val\n            $new_case_repl_escpd/" $wknetworktype_java 
    
    # WKNetworkType private static decl for new VALUE requires picking up
    # the last emitted 'next value' which is part of the commented section
    new_priv_decl_repl="/* private static final int __SYMBOL___VALUE = [0-9]{1,} */"
    new_priv_decl_repl_escpd=${new_priv_decl_repl//\*/\\*}
    last_priv_static_value=`grep -Eo "$new_priv_decl_repl_escpd" $wknetworktype_java | grep -Eo '[0-9]{1,}'`
    next_priv_static_value=$((last_priv_static_value + 1))
    next_val="private static final int ${symbol}_VALUE = ${last_priv_static_value};"
    next_priv_decl_repl="\/\* private static final int __SYMBOL___VALUE = $next_priv_static_value \*\/"
    new_priv_decl_repl_escpd="\/\* private static final int __SYMBOL___VALUE = ${last_priv_static_value} \*\/"
    sed -i '' -e "1,\$s/$new_priv_decl_repl_escpd/$next_val\n    $next_priv_decl_repl/" $wknetworktype_java 
}

# Update Java NetworkType.java and BRD Utilities.java to include new blockchain
# mnemonics
#
# @param symbol The blockchain mnemonic
update_java() {
    symbol=$1

    update_networktype_java $symbol
    update_brd_utilities_java $symbol
    update_native_wknetworktype_java $symbol
}

# ---------------------- main -------------------------
# Valid options beyond this point
get_opts "$@" 

# Resolve top level output paths
output_path=($root/WalletKitCore $root/WalletKitSwift $root/WalletKitJava)

# Check for presence of required templates
if [ ! -d $templates ] ||
   [ ! -d "$templates/${package_names[$HANDLER_PKG]}" ]  ||
   [ ! -d "$templates/${package_names[$BLOCKCHAIN_PKG]}" ]  ||
   [ ! -d "$templates/${package_names[$TESTS_PKG]}" ]  ||
   [ ! -f "$templates/$wk_config_templ" ]; then
    echo "$templates/$wk_config_templ"
    echo "ERR: Blockchain templates not found at $templates"
    exit 1
fi

# Determine user and date
gen_date=`date '+%Y-%m-%d'`
copyright_year=`date +%Y`
if [ "$gen_user" == "" ]; then
    gen_user=$USER
fi 

# Get appropriate code replacement symbols
bc_symbol=`echo $bc_symbol | tr '[:lower:]' '[:upper:]'`
bc_symbol_lc=`echo $bc_symbol | tr '[:upper:]' '[:lower:]'`

bc_name="$(tr '[:lower:]' '[:upper:]' <<< ${bc_name:0:1})${bc_name:1}"
bc_name_uc=`echo $bc_name | tr '[:lower:]' '[:upper:]'`
bc_name_lc=`echo $bc_name | tr '[:upper:]' '[:lower:]'`

echov
echov "Generating for blockchain:"
echov "  User:           $gen_user"
echov "  Symbol:         $bc_symbol ($bc_symbol_lc)"
echov "  Name:           $bc_name ($bc_name_uc $bc_name_lc)"
gens=""

for (( i=0; i<${#packages[@]}; i++ )); do
   if [ ${packages[$i]} -ne 0 ]; then 
      gens="$gens${package_names[$i]} "
   fi
done 
echov "  Gens:           $gens"
echov "  Output to:      $root"
echov "  From templates: $templates"

# Target output folders are created and pkg_paths
# is updated to contain the paths to new blockchain output paths
per_pkg_folders=($bc_symbol_lc $bc_name_lc $bc_symbol_lc)
check_create_folders ${per_pkg_folders[@]} 

# Base templates for each active package are copied & renamed
per_pkg_rename_symbols=("__SYMBOL__" $bc_symbol 
                        "__Name__"   $bc_name
                        "__Name__"   $bc_name)
copy_templates ${per_pkg_rename_symbols[@]} 

# Preliminary for all packages
pkg_enabled $HANDLER_PKG $BLOCKCHAIN_PKG $TESTS_PKG
if [ $pkg_enabled_result -eq 1 ]; then
    check_create_cmakelists   
fi
pkg_enabled $HANDLER_PKG $BLOCKCHAIN_PKG
if [ $pkg_enabled_result -eq 1 ]; then
    # check for and create a new WKConfig.h if necessary
    check_create_wkconfig
    
    # check for and create a new WKBase.h if necessary
    update_wkbase $bc_symbol $bc_symbol_lc

    # check for and create a new WKAccount.c if necessary
    update_wkaccount $bc_symbol

    # check for and create a new WKNetwork.c if necessary
    update_wknetwork $bc_symbol
fi

# Update naked templates for handlers
if [ ${packages[$HANDLER_PKG]} -ne 0 ]; then

    echov "  Creating '$bc_symbol' ($bc_name) handler"
    rename_symbols=("__USER__"   ${gen_user// /\\s}
                    "__DATE__"   $gen_date
                    "__YEAR__"   $copyright_year
                    "__SYMBOL__" $bc_symbol
                    "__symbol__" $bc_symbol_lc
                    "__NAME__"   $bc_name_uc
                    "__name__"   $bc_name_lc
                    "__Name__"   $bc_name)
    personalize_package ${pkg_paths[$HANLDER_PKG]} ${rename_symbols[@]}

    rename_symbols=("__SYMBOL__" $bc_symbol
                    "__name__"   $bc_name_lc
                    "__Name__"   $bc_name)
    update_wkconfig $wk_config_file $bc_symbol ${rename_symbols[@]}

    # Update handler portion of cmakelists
    rename_symbols=("__SYMBOL__" $bc_symbol)
    fill_sources=("__HANDLER_SOURCES__" ${pkg_paths[$HANLDER_PKG]})
    update_cmakelists ${output_path[$WKCORE_OUTPUT]} \
                      $cmakelists_file \
                      $build_hndlr_templ \
                      "__NEW_HANDLER__" \
                      ${#rename_symbols[@]} \
                      ${rename_symbols[@]} \
                      ${fill_sources[@]} 
fi

# Update naked templates for blockchain impl 
if [ ${packages[$BLOCKCHAIN_PKG]} -ne 0 ]; then
    echov "  Creating '$bc_symbol' ($bc_name) blockchain impl"
    rename_symbols=("__USER__"   ${gen_user// /\\s}
                    "__DATE__"   $gen_date
                    "__YEAR__"   $copyright_year
                    "__SYMBOL__" $bc_symbol
                    "__NAME__"   $bc_name_uc
                    "__name__"   $bc_name_lc 
                    "__Name__"   $bc_name )
    personalize_package ${pkg_paths[$BLOCKCHAIN_PKG]} ${rename_symbols[@]}
    
    # Update blockchain portion of cmakelists
    rename_symbols=("__Name__" $bc_name)
    fill_sources=("__BLOCKCHAIN_SOURCES__" ${pkg_paths[$BLOCKCHAIN_PKG]})
    update_cmakelists ${output_path[$WKCORE_OUTPUT]} \
                      $cmakelists_file \
                      $build_bc_templ \
                      "__NEW_BLOCKCHAIN_IMPL__" \
                      ${#rename_symbols[@]} \
                      ${rename_symbols[@]} \
                      ${fill_sources[@]} 
fi

# Update test template
if [ ${packages[$TESTS_PKG]} -ne 0 ]; then
    echov "  Creating '$bc_symbol' ($bc_name) test"

    rename_symbols=("__USER__"   ${gen_user// /\\s}
                    "__DATE__"   $gen_date
                    "__YEAR__"   $copyright_year
                    "__SYMBOL__" $bc_symbol
                    "__NAME__"   $bc_name_uc
                    "__name__"   $bc_name_lc 
                    "__Name__"   $bc_name )
    personalize_package ${pkg_paths[$TESTS_PKG]} ${rename_symbols[@]}
  
    # Create the test area if needed (not generating on WalletKitCore)
    # and update the test framework to include the new test 
    check_create_walletkitcoretests
    update_walletkitcoretests $bc_name

    # Update blockchain portion of cmakelists
    rename_symbols=("__Name__" $bc_name
                    "__name__" $bc_name_lc)
    fill_sources=("__TEST_SOURCES__" ${pkg_paths[$TESTS_PKG]})
    update_cmakelists ${output_path[$WKCORE_OUTPUT]} \
                      $cmakelists_file \
                      $build_test_templ \
                      "__NEW_BLOCKCHAIN_TEST__" \
                      ${#rename_symbols[@]} \
                      ${rename_symbols[@]} \
                      ${fill_sources[@]} 
fi

# Update Swift
if [ ${packages[$SWIFT_PKG]} -ne 0 ]; then
    echov "  Updating Swift for '$bc_symbol'"
    check_create_swift
    update_swift $bc_symbol $bc_symbol_lc    
fi

# Update Java
if [ ${packages[$JAVA_PKG]} -ne 0 ]; then
    echov "  Updating Java for '$bc_symbol'"
    check_create_java
    update_java $bc_symbol  
fi
