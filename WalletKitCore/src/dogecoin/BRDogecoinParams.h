//
//  BRDogecoinParams.h
//
//  Created by Aaron Voisine on 5/19/21.
//  Copyright (c) 2021 breadwallet LLC
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in
//  all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//  THE SOFTWARE.
//
//                        ,oc,
//   BAD CAT.            ,OOxoo,                                  .cl::
//                       ,OOxood,                               .lxxdod,
//       VERY CRYPTO!    :OOxoooo.                             'ddddoc:c.
//                       :kkxooool.                          .cdddddc:::o.
//                       :kkdoooool;'                      ;dxdddoooc:::l;
//                       dkdooodddddddl:;,''...         .,odcldoc:::::ccc;
//                      .kxdxkkkkkxxdddddddxxdddddoolccldol:lol:::::::colc
//                     'dkkkkkkkkkddddoddddxkkkkkxdddooolc:coo::;'',::llld
//                 .:dkkkkOOOOOkkxddoooodddxkxkkkxddddoc:::oddl:,.';:looo:
//             ':okkkkkkkOO0000Okdooodddddxxxxdxxxxdddddoc:loc;...,codool
//           'dkOOOOOOkkkO00000Oxdooddxxkkkkkkxxdddxxxdxxxooc,..';:oddlo.
//          ,kOOO0OOkOOOOOO00OOxdooddxOOOOOkkkxxdddxxxxkxxkxolc;cloolclod.
//         .kOOOO0Okd:;,cokOOkxdddddxOO0OOOOOkxddddddxkxkkkkkxxdoooollloxk'
//         l00KKKK0xl,,.',xkkkkkxxxxkOOOkkOkkkkkxddddddxkkkkkkkkxoool::ldkO'
//        '00KXXKK0oo''..ckkkkkkkOkkkkkkxl;'.':oddddddxkkkkkkkkkkkdol::codkO.
//        xKKXXK00Oxl;:lxkkkkkkOOkkddoc,'lx:'   ;lddxkkkkkkkxkkkkkxdolclodkO.
//       ;KKXXXK0kOOOOOkkkkOOOOOOkkdoc'.'o,.  ..,oxkkkOOOkkkkkkkkkkddoooodxk
//       kKXKKKKKOOO00OOO00000OOOkkxddo:;;;'';:okOO0O0000OOOOOOOOOkkxddddddx
//      .KKKKKKKKOkxxdxkkkOOO000OkkkxkkkkkxxkkkkkOO0KKKKK0OOOO000OOOkkdddddk.
//      xKKKKKKc,''''''';lx00K000OOkkkOOOkkkkkkkkO0KKKKKK0OO0000O000Okkxdkkx
//     'KK0KKXx. ..    ...'xKKKK00OOOOO000000000OO0KKKKKKKKKKKKK0OOOOOkxdkko
//     xKKKKKXx,...      .,dKXKK00000000KKKKKKKKKKKKKKKKKKKK000OOOOOOkxddxd.
//    ,KKKKKXKd'.....  ..,ck00OOOOOOkO0KKKKKKKKKKKKKKKKKK0OOOOkkkkkkkxdddo.
//    .KKKKK0xc;,......',cok0O0OOOkkkk0KKKK00000KKK000OOOkkkkkkkkkkkxdddd.
//    .KKKKK0dc;,,'''''',:oodxkkkkkkkkkOOOOkOOOOkkkkkkkkkkkkkkkOOkkxdddd,
//     0KKKKK0x;'.   ...';lodxxkkkkkkddkkkkkkkkkkkkkkkkkkOOOOOkkOkkkxddc
//     xKKKKKK0l;'........';cdolc:;;;:lkkkkkkkkkkkkkkkkOO000OOOOOOkxddd.
//     :KKKKK00Oxo:,'',''''...,,,;;:ldxkkkkkkkkkkkkkOkkOOOOOOOOkkkxddd'
//      oKKKKK0OOkxlloloooolloooodddxkkkkkkkkkkkkkkkkkkkkkkkOOkkkxddd.
//       :KKK00OO0OOkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkO0Okkkkkkkkxddd:
//        o0KK00000000OOkkkkkkkkkkkkkkkkkkkkkkkkkkO0000Okkkkkkxdo;.
//         'd00000000OOOOOOkkkkkkkkkkkkkkkkkOkOO00Okkkkkkkkkkko,
//           .oO00000OOOOOkkkkkkkkkkkkkkkkkkOOOOkOOkkkkkkkkko'
//             .;xO0OOOOOOkkkkkkkkkkkkkkkkkkkkkOkkkkkkkkd:.
//                .lxOOOOkkkkkkkkkkkkkkkkkkkxxxkkkkkd:'
//                   .;okkkkkkkkxxkkdxxddxdxdolc;'..
//                       ...',;::::::;;,'...

#ifndef BRDogecoinParams_h
#define BRDogecoinParams_h

#include "bitcoin/BRBitcoinChainParams.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DOGE_BIP32_DEPTH 3
#define DOGE_BIP32_CHILD ((const uint32_t []){ 44 | BIP32_HARD, 3 | BIP32_HARD, 0 | BIP32_HARD })

extern const BRBitcoinChainParams *dogeChainParams(bool mainnet);

static inline int dogeChainParamsHasParams (const BRBitcoinChainParams *params) {
    return dogeChainParams(true) == params || dogeChainParams(false) == params;
}

#ifdef __cplusplus
}
#endif

#endif // BRDogecoinParams_h
