#!/usr/bin/env bash
#
#  make_blockchain.sh 
#  WalletKitCore
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
#   bc_name:            A short descriptive name for the blockchain (e.g. Tezos
#   packages:           Default packages to be produced for the new blockchain; handlers & blockchain
#                       implementation 
#   output_path:        Output folder for blockchain 'instantiation'
#   output_pkg_paths    Initial paths within src folders for output blockchain code
#   pkg_paths           Working directories based on customized output_pkg_paths 
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
packages=(1 1)
package_names=("handlers" "blockchain")
output_path="../WalletKitCore"
output_pkg_paths=("walletkit/handlers" ".")
pkg_paths=()
templates="../templates"
wk_config_templ="WkConfig.h.tmpl"
build_bc_templ="cmakelists.bc.tmpl"
build_hndlr_templ="cmakelists.hndlr.tmpl"
include_testnet=0
verbose=0

HANDLER_PKG=0
BLOCKCHAIN_PKG=1

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
    echo "          g: Specifies a code package index to include, and controls which type of"
    echo "             output will be generated:"
    echo "                0: handlers package"
    echo "                1: blockchain implementation package"
    echo "             By default all packages are generated"
    echo
    echo "          o: Specifies the output folder for generated code" 
    echo "          i: Path to templates"
    echo "          v: Verbose output"
    echo "          t: Creates header files supporting testnet (default mainnet)"
    echo "          u: Username for decorating copyright notices (otherwise shell defaults)"
}

# Gets all program options and populates global variables.
#
# Requires at least two arguments containing the blockchain symbol
# and blockname name.
#
# @param bc_symbol The blockchain mnemonic 
# @param bc_name The name of the new blockchain
get_opts() {

    custom_pkgs=(0 0)
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
            output_path=$OPTARG
            parm_num=$(($parm_num+1))
            ;;

        u)
            gen_user=$OPTARG
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

# Creates output folders for created packages
# This function also updates the pkg_paths for each package type (handler/blockchain)
# with the specific folder names being created for this blockchain
#
# param path: The first argument which is the main output path (e.g. 'src')
# param bc_folder: The second argument which is the new blockchain specific folder name
check_create_folders() {
    
    path=$1
    bc_folder=$2

    if [ ! -d $path ]; then
        echov
        echov "  Creating output folder: $path"
        mkdir -p $path/src
    fi

    for (( i=0; i<${#packages[@]}; i++)); do
        if [ ${packages[$i]} -ne 0 ]; then

            # Remember the final new output package path, per package type
            pkg_paths[$i]=$path/src/${output_pkg_paths[$i]}/$bc_folder
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
    for (( i=0; i<${#packages[@]}; i++)); do
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
            sed -i '' -e "1,\$s/${symbols[replace]}/${symbols[replace+1]}/g" $pkg_path/$pkg_file
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

    mfile=$1
    tmpl=$2
    repl=$3
    num_symbols=$4
    symbols=(${@:5:$num_symbols})
    fills=(${@:$((5 + $num_symbols))})

    # Start with bare template and replace symbols
    cmake=`cat $templates/$tmpl`
    for (( i=0; i<${#symbols[@]}; i+=2 )); do
        cmake=${cmake//${symbols[i]}/${symbols[i+1]}}
    done

    for (( i=0; i<${#fills[@]}; i+=2 )); do
        
        # Create the list of buildable files
        # and substitute the list into the template
        files=`ls ${fills[i+1]}`
        cmake=${cmake//${fills[i]}/$files}
        cmake=${cmake//$'\n'/\\n}

        # Substitute the templace section into
        # the makefile
        sed -i '' -e "s/$repl/$cmake/" $mfile
    done
}

# Checks if there is a cmakelists in the target output
# folder and if not, creates one
function check_create_cmakelists() {
    # Update build files for new handler
    cmakelists_file=$output_path/CMakeLists.txt
    if [ ! -f $cmakelists_file ]; then
        echov "  Creating new CMakeLists.txt file"
        touch $cmakelists_file
        echo $'#' __NEW_BLOCKCHAIN_IMPL__$'\n'$'\n' >> $cmakelists_file
        echo $'#' __NEW_HANDLER__$'\n'$'\n' >> $cmakelists_file
    fi
}

# Checks if there is a WKConfig.h in the target output
# folder and if not, creates one
function check_create_wkconfig() {
    wk_config_file=$output_path/src/walletkit/WKConfig.h
    if [ ! -f $wk_config_file ]; then
        echov "  Creating new WKConfig.h file"
        touch $wk_config_file
        echo // __CONTROL__$'\n'$'\n'// __CONFIG__$'\n'$'\n' >> $wk_config_file
    fi
}

# Checks if there is a WKBase.h in the target output
# folder and if not, creates one
function check_create_wkbase() {
    wk_base_file=$output_path/include/WKBase.h
    if [ ! -f $wk_base_file ]; then
        echov "  Creating new WKBase.h file"
        mkdir -p $output_path/include
        touch $wk_base_file
        
        echo $'#'ifndef WKBase.h$'\n'$'#'define WKBase.h$'\n' >> $wk_base_file
        echo typedef enum {$'\n''    '/* WK_NETWORK_TYPE___SYMBOL__' '*/$'\n' >> $wk_base_file
        echo '    'WK_NETWORK_NUMBER_OF_NETWORK_TYPES$'\n'} WKNetworkType$';'$'\n' >> $wk_base_file
        echo "/* #define WK_NETWORK_CURRENCY___SYMBOL__    \"__symbol__\" */"$'\n' >> $wk_base_file
        echo $'#'endif$'\n' >> $wk_base_file
    fi
}

# Updates the WKBase.h header file with required definitions
#
# @param ucsymbol The UpperCase blockchain mnemonic
# @param lcsymbol The lowercase blockchain mnemonic
function update_wkbase() {

    ucsymbol=$1
    lcsymbol=$2

    wk_base_file=$output_path/include/WKBase.h

    # Create if necessary    
    check_create_wkbase

    net_type_repl="/* WK_NETWORK_TYPE___SYMBOL__ */" 
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
    sed -i '' -e "1,\$s/$net_type_repl/$net_type,\n    $net_type_repl/" $wk_base_file 
    sed -i '' -e "1,\$s/$net_currency_repl/$net_currency\n$net_currency_repl/" $wk_base_file 
}

# ---------------------- main -------------------------
# Valid options beyond this point
get_opts $*

# Check for presence of required templates
if [ ! -d $templates ] ||
   [ ! -d "$templates/${package_names[$HANDLER_PKG]}" ]  ||
   [ ! -d "$templates/${package_names[$BLOCKCHAIN_PKG]}" ]  ||
   [ ! -f "$templates/$wk_config_templ" ]; then
    echo "$templates/$wk_config_templ"
    echo "ERR: Blockchain templates not found at $templates"
    exit 1
fi

# Determine user and date
gen_date=`date '+%Y-%m-%d'`
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
echov "  Output to:      $output_path"
echov "  From templates: $templates"

# Target output folders are created and pkg_paths
# is updated to contain the paths to new blockchain output paths
check_create_folders $output_path $bc_symbol_lc

# Base templates for each active component are copied & renamed
rename_symbols=("__SYMBOL__" $bc_symbol 
                "__Name__"   $bc_name)
copy_templates ${rename_symbols[@]} 

# create a cmakelists if it does not exist
check_create_cmakelists   

# check for and create a new WKConfig.h if necessary
check_create_wkconfig

# check for and create a new WKBase.h if necessary
update_wkbase $bc_symbol $bc_symbol_lc

# Update naked templates for handlers
if [ ${packages[$HANDLER_PKG]} -ne 0 ]; then

    echov "  Creating '$bc_symbol' ($bc_name) handler"
    rename_symbols=("__USER__"   $gen_user
                    "__DATE__"   $gen_date
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
    update_cmakelists $cmakelists_file \
                      $build_hndlr_templ \
                      "__NEW_HANDLER__" \
                      ${#rename_symbols[@]} \
                      ${rename_symbols[@]} \
                      ${fill_sources[@]} 
fi

# Update naked templates for blockchain impl 
if [ ${packages[$BLOCKCHAIN_PKG]} -ne 0 ]; then
    echov "  Creating '$bc_symbol' ($bc_name) blockchain impl"
    rename_symbols=("__USER__"   $gen_user
                    "__DATE__"   $gen_date
                    "__SYMBOL__" $bc_symbol
                    "__NAME__"   $bc_name_uc
                    "__name__"   $bc_name_lc 
                    "__Name__"   $bc_name )
    personalize_package ${pkg_paths[$BLOCKCHAIN_PKG]} ${rename_symbols[@]}
    
    # Update blockchain portion of cmakelists
    rename_symbols=("__Name__" $bc_name)
    fill_sources=("__BLOCKCHAIN_SOURCES__" ${pkg_paths[$BLOCKCHAIN_PKG]})
    update_cmakelists $cmakelists_file \
                      $build_bc_templ \
                      "__NEW_BLOCKCHAIN_IMPL__" \
                      ${#rename_symbols[@]} \
                      ${rename_symbols[@]} \
                      ${fill_sources[@]} 
fi
