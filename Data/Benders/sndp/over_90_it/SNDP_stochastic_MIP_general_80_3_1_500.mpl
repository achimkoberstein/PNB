!   
!   
    SNDP_stochastic_MIP_general_instances\SNDP_stochastic_MIP_general_80_3_1_500.mpl
!   
!   Generated with the MPL Modeling System
!   Copyright (c) 1988-2016  Maximal Software
!   
!   Date:               October 16, 2018
!   Time:               18:16
!   
!   Constraints:        193
!   Variables:          1886
!   Integers:           64
!   Nonzeros:           1475
!   Density:            0.4 %
!   

TITLE SNDP_stochastic_general ;

DECISION VARIABLES

    OpenProduction01 OpenProduction02 OpenProduction03 
    OpenProduction04 OpenProduction05 OpenProduction06 
    OpenProduction07 OpenProduction08 OpenProduction09 
    OpenProduction10 OpenProduction11 OpenProduction12 
    OpenProduction13 OpenProduction14 OpenProduction16 
    OpenProduction17 OpenProduction18 OpenProduction19 
    OpenProduction20 OpenProduction21 OpenProduction22 
    OpenProduction23 OpenProduction24 OpenProduction25 
    OpenProduction28 OpenProduction29 OpenProduction30 
    OpenProduction31 OpenProduction33 OpenProduction35 
    OpenProduction36 OpenProduction37 OpenProduction39 
    OpenProduction41 OpenProduction42 OpenProduction44 
    OpenProduction46 OpenProduction47 OpenProduction48 
    OpenProduction49 OpenProduction50 OpenProduction52 
    OpenProduction53 OpenProduction54 OpenProduction55 
    OpenProduction56 OpenProduction57 OpenProduction58 
    OpenProduction59 OpenProduction60 OpenProduction62 
    OpenProduction63 OpenProduction64 OpenProduction65 
    OpenProduction66 OpenProduction68 OpenProduction69 
    OpenProduction70 OpenProduction71 OpenProduction72 
    OpenProduction73 OpenProduction75 OpenProduction76 
    OpenProduction77 Ship10101 Ship10102 Ship10103 Ship10106 Ship10107 
    Ship10109 Ship10111 Ship10112 Ship10113 Ship10116 Ship10119 
    Ship10120 Ship10121 Ship10135 Ship10136 Ship10137 Ship10142 
    Ship10148 Ship10152 Ship10156 Ship10165 Ship10173 Ship10175 
    Ship10176 Ship10504 Ship10505 Ship10510 Ship10511 Ship10514 
    Ship10520 Ship10523 Ship10525 Ship10547 Ship10552 Ship10553 
    Ship10555 Ship10559 Ship10568 Ship10570 Ship10573 Ship11102 
    Ship11103 Ship11106 Ship11108 Ship11109 Ship11110 Ship11111 
    Ship11114 Ship11117 Ship11120 Ship11121 Ship11123 Ship11125 
    Ship11128 Ship11130 Ship11131 Ship11133 Ship11141 Ship11142 
    Ship11144 Ship11146 Ship11147 Ship11148 Ship11152 Ship11153 
    Ship11156 Ship11157 Ship11160 Ship11162 Ship11163 Ship11164 
    Ship11168 Ship11169 Ship11171 Ship11172 Ship11173 Ship11176 
    Ship11205 Ship11207 Ship11208 Ship11209 Ship11212 Ship11218 
    Ship11220 Ship11223 Ship11225 Ship11235 Ship11242 Ship11249 
    Ship11254 Ship11256 Ship11257 Ship11260 Ship11262 Ship11263 
    Ship11269 Ship11275 Ship11414 Ship11418 Ship11424 Ship11425 
    Ship11435 Ship11436 Ship11447 Ship11449 Ship11466 Ship11468 
    Ship11471 Ship11535 Ship11537 Ship11542 Ship11555 Ship11563 
    Ship11571 Ship11576 Ship12102 Ship12105 Ship12107 Ship12113 
    Ship12120 Ship12121 Ship12123 Ship12125 Ship12128 Ship12129 
    Ship12131 Ship12133 Ship12135 Ship12147 Ship12148 Ship12149 
    Ship12150 Ship12156 Ship12171 Ship12173 Ship12175 Ship12403 
    Ship12405 Ship12406 Ship12407 Ship12408 Ship12409 Ship12410 
    Ship12411 Ship12413 Ship12417 Ship12422 Ship12423 Ship12424 
    Ship12429 Ship12431 Ship12433 Ship12435 Ship12447 Ship12450 
    Ship12455 Ship12456 Ship12465 Ship12466 Ship12468 Ship12471 
    Ship12472 Ship12473 Ship12475 Ship12477 Ship13003 Ship13007 
    Ship13008 Ship13009 Ship13010 Ship13013 Ship13019 Ship13025 
    Ship13028 Ship13029 Ship13030 Ship13031 Ship13035 Ship13037 
    Ship13044 Ship13046 Ship13047 Ship13048 Ship13049 Ship13050 
    Ship13052 Ship13054 Ship13055 Ship13056 Ship13058 Ship13060 
    Ship13062 Ship13064 Ship13066 Ship13068 Ship13070 Ship13072 
    Ship13075 Ship13076 Ship13077 Ship13402 Ship13407 Ship13411 
    Ship13439 Ship13460 Ship14001 Ship14002 Ship14003 Ship14004 
    Ship14005 Ship14006 Ship14007 Ship14008 Ship14009 Ship14010 
    Ship14012 Ship14014 Ship14016 Ship14017 Ship14018 Ship14019 
    Ship14020 Ship14021 Ship14022 Ship14023 Ship14024 Ship14025 
    Ship14028 Ship14029 Ship14030 Ship14031 Ship14033 Ship14035 
    Ship14036 Ship14037 Ship14039 Ship14041 Ship14044 Ship14046 
    Ship14048 Ship14049 Ship14050 Ship14052 Ship14053 Ship14054 
    Ship14055 Ship14056 Ship14057 Ship14058 Ship14059 Ship14060 
    Ship14062 Ship14063 Ship14066 Ship14068 Ship14069 Ship14070 
    Ship14071 Ship14073 Ship14075 Ship14076 Ship14077 Ship14101 
    Ship14103 Ship14104 Ship14105 Ship14109 Ship14112 Ship14113 
    Ship14114 Ship14117 Ship14119 Ship14136 Ship14137 Ship14139 
    Ship14141 Ship14144 Ship14148 Ship14149 Ship14150 Ship14153 
    Ship14155 Ship14157 Ship14158 Ship14160 Ship14162 Ship14163 
    Ship14166 Ship14169 Ship14171 Ship14173 Ship14175 Ship14176 
    Ship14204 Ship14205 Ship14206 Ship14218 Ship14237 Ship14242 
    Ship14249 Ship14252 Ship14264 Ship14266 Ship14268 Ship14301 
    Ship14302 Ship14303 Ship14304 Ship14305 Ship14310 Ship14311 
    Ship14314 Ship14316 Ship14319 Ship14322 Ship14330 Ship14337 
    Ship14342 Ship14344 Ship14355 Ship14356 Ship14357 Ship14358 
    Ship14363 Ship14364 Ship14366 Ship14369 Ship14371 Ship14372 
    Ship14373 Ship14401 Ship14404 Ship14406 Ship14407 Ship14409 
    Ship14410 Ship14414 Ship14418 Ship14419 Ship14422 Ship14423 
    Ship14428 Ship14433 Ship14437 Ship14439 Ship14444 Ship14446 
    Ship14447 Ship14449 Ship14450 Ship14453 Ship14455 Ship14457 
    Ship14458 Ship14459 Ship14465 Ship14472 Ship14473 Ship14475 
    Ship14508 Ship14511 Ship14512 Ship14514 Ship14516 Ship14518 
    Ship14521 Ship14529 Ship14530 Ship14544 Ship14547 Ship14548 
    Ship14549 Ship14554 Ship14556 Ship14557 Ship14558 Ship14559 
    Ship14562 Ship14565 Ship14569 Ship14570 Ship14572 Ship14576 
    Ship14577 Ship14703 Ship14704 Ship14706 Ship14709 Ship14713 
    Ship14716 Ship14717 Ship14718 Ship14719 Ship14720 Ship14723 
    Ship14725 Ship14733 Ship14737 Ship14739 Ship14742 Ship14746 
    Ship14747 Ship14749 Ship14750 Ship14755 Ship14757 Ship14758 
    Ship14760 Ship14762 Ship14763 Ship14764 Ship14765 Ship14768 
    Ship14769 Ship14771 Ship14772 Ship14777 Ship15010 Ship15014 
    Ship15017 Ship15035 Ship15036 Ship15037 Ship15050 Ship15054 
    Ship15057 Ship15062 Ship15064 Ship15066 Ship15068 Ship15072 
    Ship15073 Ship15075 Ship16013 Ship16014 Ship16019 Ship16020 
    Ship16024 Ship16031 Ship16037 Ship16039 Ship16048 Ship16053 
    Ship16058 Ship16060 Ship16062 Ship16072 Ship16103 Ship16104 
    Ship16105 Ship16106 Ship16108 Ship16111 Ship16114 Ship16119 
    Ship16122 Ship16124 Ship16128 Ship16129 Ship16135 Ship16136 
    Ship16142 Ship16144 Ship16149 Ship16153 Ship16154 Ship16155 
    Ship16156 Ship16157 Ship16159 Ship16164 Ship16166 Ship16168 
    Ship16171 Ship16177 Ship16701 Ship16709 Ship16729 Ship16753 
    Ship16756 Ship16773 Ship17901 Ship17902 Ship17903 Ship17904 
    Ship17905 Ship17907 Ship17908 Ship17909 Ship17910 Ship17911 
    Ship17913 Ship17914 Ship17917 Ship17918 Ship17919 Ship17920 
    Ship17921 Ship17922 Ship17923 Ship17924 Ship17925 Ship17928 
    Ship17929 Ship17930 Ship17931 Ship17933 Ship17935 Ship17936 
    Ship17937 Ship17939 Ship17941 Ship17942 Ship17944 Ship17946 
    Ship17947 Ship17949 Ship17950 Ship17952 Ship17953 Ship17954 
    Ship17955 Ship17958 Ship17959 Ship17962 Ship17963 Ship17964 
    Ship17965 Ship17968 Ship17969 Ship17970 Ship17973 Ship17975 
    Ship17976 Ship20202 Ship20203 Ship20206 Ship20208 Ship20209 
    Ship20210 Ship20212 Ship20214 Ship20217 Ship20218 Ship20219 
    Ship20220 Ship20222 Ship20223 Ship20224 Ship20225 Ship20228 
    Ship20229 Ship20231 Ship20235 Ship20237 Ship20239 Ship20241 
    Ship20242 Ship20244 Ship20246 Ship20248 Ship20252 Ship20254 
    Ship20257 Ship20258 Ship20259 Ship20263 Ship20264 Ship20265 
    Ship20266 Ship20268 Ship20269 Ship20270 Ship20271 Ship20275 
    Ship20276 Ship21602 Ship21603 Ship21605 Ship21606 Ship21607 
    Ship21610 Ship21611 Ship21613 Ship21616 Ship21621 Ship21622 
    Ship21624 Ship21625 Ship21628 Ship21633 Ship21635 Ship21637 
    Ship21641 Ship21648 Ship21655 Ship21657 Ship21662 Ship21664 
    Ship21665 Ship21666 Ship21668 Ship21669 Ship21670 Ship21672 
    Ship21675 Ship22307 Ship22308 Ship22309 Ship22314 Ship22317 
    Ship22318 Ship22323 Ship22329 Ship22330 Ship22331 Ship22337 
    Ship22341 Ship22342 Ship22348 Ship22350 Ship22352 Ship22362 
    Ship22364 Ship22368 Ship22372 Ship22373 Ship22375 Ship22601 
    Ship22602 Ship22605 Ship22609 Ship22610 Ship22618 Ship22620 
    Ship22623 Ship22624 Ship22635 Ship22637 Ship22642 Ship22647 
    Ship22654 Ship22656 Ship22664 Ship22673 Ship22703 Ship22705 
    Ship22706 Ship22707 Ship22709 Ship22716 Ship22720 Ship22722 
    Ship22723 Ship22724 Ship22731 Ship22733 Ship22735 Ship22741 
    Ship22742 Ship22744 Ship22746 Ship22749 Ship22750 Ship22753 
    Ship22754 Ship22758 Ship22759 Ship22760 Ship22762 Ship22763 
    Ship22765 Ship22769 Ship22770 Ship22773 Ship22777 Ship23101 
    Ship23103 Ship23104 Ship23105 Ship23107 Ship23109 Ship23110 
    Ship23112 Ship23113 Ship23116 Ship23118 Ship23119 Ship23120 
    Ship23125 Ship23128 Ship23129 Ship23131 Ship23133 Ship23135 
    Ship23136 Ship23137 Ship23139 Ship23141 Ship23144 Ship23146 
    Ship23147 Ship23149 Ship23150 Ship23152 Ship23153 Ship23154 
    Ship23155 Ship23159 Ship23162 Ship23164 Ship23165 Ship23166 
    Ship23168 Ship23170 Ship23171 Ship23172 Ship23173 Ship23175 
    Ship23176 Ship23177 Ship23201 Ship23203 Ship23205 Ship23206 
    Ship23208 Ship23210 Ship23213 Ship23216 Ship23217 Ship23218 
    Ship23219 Ship23222 Ship23225 Ship23229 Ship23235 Ship23236 
    Ship23244 Ship23246 Ship23248 Ship23249 Ship23250 Ship23252 
    Ship23253 Ship23257 Ship23266 Ship23269 Ship23270 Ship23271 
    Ship23272 Ship23275 Ship23402 Ship23407 Ship23411 Ship23439 
    Ship23460 Ship23801 Ship23804 Ship23805 Ship23808 Ship23811 
    Ship23812 Ship23814 Ship23816 Ship23818 Ship23820 Ship23823 
    Ship23828 Ship23829 Ship23831 Ship23836 Ship23841 Ship23842 
    Ship23844 Ship23849 Ship23855 Ship23857 Ship23859 Ship23862 
    Ship23863 Ship23864 Ship23865 Ship23866 Ship23868 Ship23869 
    Ship23871 Ship23872 Ship23873 Ship23876 Ship24508 Ship24511 
    Ship24512 Ship24514 Ship24516 Ship24518 Ship24521 Ship24529 
    Ship24530 Ship24544 Ship24547 Ship24548 Ship24549 Ship24554 
    Ship24556 Ship24557 Ship24558 Ship24559 Ship24562 Ship24565 
    Ship24569 Ship24570 Ship24572 Ship24576 Ship24577 Ship25101 
    Ship25103 Ship25104 Ship25105 Ship25109 Ship25111 Ship25112 
    Ship25116 Ship25117 Ship25119 Ship25120 Ship25121 Ship25123 
    Ship25124 Ship25125 Ship25128 Ship25129 Ship25130 Ship25133 
    Ship25135 Ship25136 Ship25137 Ship25139 Ship25141 Ship25144 
    Ship25147 Ship25148 Ship25149 Ship25150 Ship25152 Ship25153 
    Ship25154 Ship25155 Ship25156 Ship25157 Ship25158 Ship25159 
    Ship25160 Ship25162 Ship25163 Ship25164 Ship25166 Ship25170 
    Ship25171 Ship25173 Ship25175 Ship25177 Ship25502 Ship25503 
    Ship25504 Ship25506 Ship25508 Ship25509 Ship25512 Ship25514 
    Ship25519 Ship25525 Ship25529 Ship25536 Ship25537 Ship25539 
    Ship25542 Ship25548 Ship25549 Ship25550 Ship25554 Ship25555 
    Ship25556 Ship25557 Ship25560 Ship25563 Ship25573 Ship25575 
    Ship25576 Ship25577 Ship25903 Ship25904 Ship25909 Ship25911 
    Ship25918 Ship25922 Ship25942 Ship25946 Ship25948 Ship25955 
    Ship25956 Ship25957 Ship25959 Ship25972 Ship25976 Ship25977 
    Ship26103 Ship26104 Ship26105 Ship26106 Ship26108 Ship26111 
    Ship26114 Ship26119 Ship26122 Ship26124 Ship26128 Ship26129 
    Ship26135 Ship26136 Ship26142 Ship26144 Ship26149 Ship26153 
    Ship26154 Ship26155 Ship26156 Ship26157 Ship26159 Ship26164 
    Ship26166 Ship26168 Ship26171 Ship26177 Ship26301 Ship26304 
    Ship26305 Ship26306 Ship26307 Ship26308 Ship26309 Ship26310 
    Ship26313 Ship26314 Ship26316 Ship26317 Ship26318 Ship26319 
    Ship26320 Ship26322 Ship26323 Ship26325 Ship26328 Ship26329 
    Ship26331 Ship26333 Ship26335 Ship26336 Ship26337 Ship26339 
    Ship26342 Ship26344 Ship26346 Ship26348 Ship26349 Ship26352 
    Ship26354 Ship26356 Ship26357 Ship26358 Ship26359 Ship26360 
    Ship26362 Ship26363 Ship26364 Ship26365 Ship26369 Ship26371 
    Ship26372 Ship26373 Ship26375 Ship26377 Ship26901 Ship26903 
    Ship26904 Ship26906 Ship26907 Ship26908 Ship26909 Ship26910 
    Ship26913 Ship26914 Ship26917 Ship26918 Ship26919 Ship26920 
    Ship26921 Ship26922 Ship26923 Ship26924 Ship26925 Ship26928 
    Ship26929 Ship26930 Ship26931 Ship26933 Ship26935 Ship26936 
    Ship26937 Ship26939 Ship26942 Ship26944 Ship26946 Ship26948 
    Ship26949 Ship26950 Ship26952 Ship26953 Ship26954 Ship26955 
    Ship26956 Ship26957 Ship26958 Ship26960 Ship26962 Ship26964 
    Ship26965 Ship26966 Ship26968 Ship26969 Ship26970 Ship26971 
    Ship26972 Ship26973 Ship26975 Ship26976 Ship26977 Ship27401 
    Ship27402 Ship27404 Ship27405 Ship27407 Ship27408 Ship27409 
    Ship27414 Ship27416 Ship27420 Ship27421 Ship27423 Ship27424 
    Ship27429 Ship27436 Ship27437 Ship27439 Ship27444 Ship27448 
    Ship27449 Ship27450 Ship27452 Ship27453 Ship27454 Ship27455 
    Ship27456 Ship27457 Ship27458 Ship27460 Ship27462 Ship27463 
    Ship27464 Ship27465 Ship27466 Ship27468 Ship27471 Ship27472 
    Ship27473 Ship27476 Ship27702 Ship27710 Ship27711 Ship27716 
    Ship27719 Ship27733 Ship27736 Ship27742 Ship27744 Ship27753 
    Ship27765 Ship27772 Ship27777 Ship27801 Ship27805 Ship27811 
    Ship27821 Ship27829 Ship27830 Ship27836 Ship27837 Ship27847 
    Ship27850 Ship27852 Ship27857 Ship27864 Ship27873 Ship27901 
    Ship27902 Ship27903 Ship27904 Ship27905 Ship27907 Ship27908 
    Ship27909 Ship27910 Ship27911 Ship27913 Ship27914 Ship27917 
    Ship27918 Ship27919 Ship27920 Ship27921 Ship27922 Ship27923 
    Ship27924 Ship27925 Ship27928 Ship27929 Ship27930 Ship27931 
    Ship27933 Ship27935 Ship27936 Ship27937 Ship27939 Ship27941 
    Ship27942 Ship27944 Ship27946 Ship27947 Ship27949 Ship27950 
    Ship27952 Ship27953 Ship27954 Ship27955 Ship27958 Ship27959 
    Ship27962 Ship27963 Ship27964 Ship27965 Ship27968 Ship27969 
    Ship27970 Ship27973 Ship27975 Ship27976 Ship30102 Ship30103 
    Ship30106 Ship30107 Ship30109 Ship30111 Ship30112 Ship30113 
    Ship30116 Ship30119 Ship30120 Ship30121 Ship30135 Ship30136 
    Ship30137 Ship30142 Ship30148 Ship30152 Ship30156 Ship30165 
    Ship30173 Ship30175 Ship30176 Ship30180 Ship30203 Ship30206 
    Ship30208 Ship30209 Ship30210 Ship30212 Ship30214 Ship30217 
    Ship30218 Ship30219 Ship30220 Ship30222 Ship30223 Ship30224 
    Ship30225 Ship30228 Ship30229 Ship30231 Ship30235 Ship30237 
    Ship30239 Ship30241 Ship30242 Ship30244 Ship30246 Ship30248 
    Ship30252 Ship30254 Ship30257 Ship30258 Ship30259 Ship30263 
    Ship30264 Ship30265 Ship30266 Ship30268 Ship30269 Ship30270 
    Ship30271 Ship30275 Ship30276 Ship30280 Ship30380 Ship30480 
    Ship30504 Ship30510 Ship30511 Ship30514 Ship30520 Ship30523 
    Ship30525 Ship30547 Ship30552 Ship30553 Ship30555 Ship30559 
    Ship30568 Ship30570 Ship30573 Ship30580 Ship30680 Ship30780 
    Ship30880 Ship30980 Ship31080 Ship31102 Ship31103 Ship31106 
    Ship31108 Ship31109 Ship31110 Ship31114 Ship31117 Ship31120 
    Ship31121 Ship31123 Ship31125 Ship31128 Ship31130 Ship31131 
    Ship31133 Ship31141 Ship31142 Ship31144 Ship31146 Ship31147 
    Ship31148 Ship31152 Ship31153 Ship31156 Ship31157 Ship31160 
    Ship31162 Ship31163 Ship31164 Ship31168 Ship31169 Ship31171 
    Ship31172 Ship31173 Ship31176 Ship31180 Ship31205 Ship31207 
    Ship31208 Ship31209 Ship31218 Ship31220 Ship31223 Ship31225 
    Ship31235 Ship31242 Ship31249 Ship31254 Ship31256 Ship31257 
    Ship31260 Ship31262 Ship31263 Ship31269 Ship31275 Ship31280 
    Ship31380 Ship31418 Ship31424 Ship31425 Ship31435 Ship31436 
    Ship31447 Ship31449 Ship31466 Ship31468 Ship31471 Ship31480 
    Ship31602 Ship31603 Ship31605 Ship31606 Ship31607 Ship31610 
    Ship31611 Ship31613 Ship31621 Ship31622 Ship31624 Ship31625 
    Ship31628 Ship31633 Ship31635 Ship31637 Ship31641 Ship31648 
    Ship31655 Ship31657 Ship31662 Ship31664 Ship31665 Ship31666 
    Ship31668 Ship31669 Ship31670 Ship31672 Ship31675 Ship31680 
    Ship31780 Ship31880 Ship31980 Ship32080 Ship32102 Ship32105 
    Ship32107 Ship32113 Ship32120 Ship32123 Ship32125 Ship32128 
    Ship32129 Ship32131 Ship32133 Ship32135 Ship32147 Ship32148 
    Ship32149 Ship32150 Ship32156 Ship32171 Ship32173 Ship32175 
    Ship32180 Ship32280 Ship32307 Ship32308 Ship32309 Ship32314 
    Ship32317 Ship32318 Ship32329 Ship32330 Ship32331 Ship32337 
    Ship32341 Ship32342 Ship32348 Ship32350 Ship32352 Ship32362 
    Ship32364 Ship32368 Ship32372 Ship32373 Ship32375 Ship32380 
    Ship32403 Ship32405 Ship32406 Ship32407 Ship32408 Ship32409 
    Ship32410 Ship32411 Ship32413 Ship32417 Ship32422 Ship32423 
    Ship32429 Ship32431 Ship32433 Ship32435 Ship32447 Ship32450 
    Ship32455 Ship32456 Ship32465 Ship32466 Ship32468 Ship32471 
    Ship32472 Ship32473 Ship32475 Ship32477 Ship32480 Ship32580 
    Ship32880 Ship32980 Ship33003 Ship33007 Ship33008 Ship33009 
    Ship33010 Ship33013 Ship33019 Ship33025 Ship33028 Ship33029 
    Ship33031 Ship33035 Ship33037 Ship33044 Ship33046 Ship33047 
    Ship33048 Ship33049 Ship33050 Ship33052 Ship33054 Ship33055 
    Ship33056 Ship33058 Ship33060 Ship33062 Ship33064 Ship33066 
    Ship33068 Ship33070 Ship33072 Ship33075 Ship33076 Ship33077 
    Ship33080 Ship33101 Ship33103 Ship33104 Ship33105 Ship33107 
    Ship33109 Ship33110 Ship33112 Ship33113 Ship33116 Ship33118 
    Ship33119 Ship33120 Ship33125 Ship33128 Ship33129 Ship33133 
    Ship33135 Ship33136 Ship33137 Ship33139 Ship33141 Ship33144 
    Ship33146 Ship33147 Ship33149 Ship33150 Ship33152 Ship33153 
    Ship33154 Ship33155 Ship33159 Ship33162 Ship33164 Ship33165 
    Ship33166 Ship33168 Ship33170 Ship33171 Ship33172 Ship33173 
    Ship33175 Ship33176 Ship33177 Ship33180 Ship33380 Ship33580 
    Ship33680 Ship33780 Ship33980 Ship34101 Ship34103 Ship34104 
    Ship34105 Ship34109 Ship34112 Ship34113 Ship34114 Ship34117 
    Ship34119 Ship34136 Ship34137 Ship34139 Ship34144 Ship34148 
    Ship34149 Ship34150 Ship34153 Ship34155 Ship34157 Ship34158 
    Ship34160 Ship34162 Ship34163 Ship34166 Ship34169 Ship34171 
    Ship34173 Ship34175 Ship34176 Ship34180 Ship34204 Ship34205 
    Ship34206 Ship34218 Ship34237 Ship34249 Ship34252 Ship34264 
    Ship34266 Ship34268 Ship34280 Ship34401 Ship34404 Ship34406 
    Ship34407 Ship34409 Ship34410 Ship34414 Ship34418 Ship34419 
    Ship34422 Ship34423 Ship34428 Ship34433 Ship34437 Ship34439 
    Ship34446 Ship34447 Ship34449 Ship34450 Ship34453 Ship34455 
    Ship34457 Ship34458 Ship34459 Ship34465 Ship34472 Ship34473 
    Ship34475 Ship34480 Ship34680 Ship34703 Ship34704 Ship34706 
    Ship34709 Ship34713 Ship34716 Ship34717 Ship34718 Ship34719 
    Ship34720 Ship34723 Ship34725 Ship34733 Ship34737 Ship34739 
    Ship34742 Ship34746 Ship34749 Ship34750 Ship34755 Ship34757 
    Ship34758 Ship34760 Ship34762 Ship34763 Ship34764 Ship34765 
    Ship34768 Ship34769 Ship34771 Ship34772 Ship34777 Ship34780 
    Ship34880 Ship34980 Ship35010 Ship35014 Ship35017 Ship35035 
    Ship35036 Ship35037 Ship35054 Ship35057 Ship35062 Ship35064 
    Ship35066 Ship35068 Ship35072 Ship35073 Ship35075 Ship35080 
    Ship35280 Ship35380 Ship35480 Ship35502 Ship35503 Ship35504 
    Ship35506 Ship35508 Ship35509 Ship35512 Ship35514 Ship35519 
    Ship35525 Ship35529 Ship35536 Ship35537 Ship35539 Ship35542 
    Ship35548 Ship35549 Ship35550 Ship35554 Ship35556 Ship35557 
    Ship35560 Ship35563 Ship35573 Ship35575 Ship35576 Ship35577 
    Ship35580 Ship35680 Ship35780 Ship35880 Ship35903 Ship35904 
    Ship35909 Ship35911 Ship35918 Ship35922 Ship35942 Ship35946 
    Ship35948 Ship35955 Ship35956 Ship35957 Ship35972 Ship35976 
    Ship35977 Ship35980 Ship36013 Ship36014 Ship36019 Ship36020 
    Ship36024 Ship36031 Ship36037 Ship36039 Ship36048 Ship36053 
    Ship36058 Ship36062 Ship36072 Ship36080 Ship36280 Ship36301 
    Ship36304 Ship36305 Ship36306 Ship36307 Ship36308 Ship36309 
    Ship36310 Ship36313 Ship36314 Ship36316 Ship36317 Ship36318 
    Ship36319 Ship36320 Ship36322 Ship36323 Ship36325 Ship36328 
    Ship36329 Ship36331 Ship36333 Ship36335 Ship36336 Ship36337 
    Ship36339 Ship36342 Ship36344 Ship36346 Ship36348 Ship36349 
    Ship36352 Ship36354 Ship36356 Ship36357 Ship36358 Ship36359 
    Ship36360 Ship36362 Ship36364 Ship36365 Ship36369 Ship36371 
    Ship36372 Ship36373 Ship36375 Ship36377 Ship36380 Ship36480 
    Ship36580 Ship36680 Ship36880 Ship36901 Ship36903 Ship36904 
    Ship36906 Ship36907 Ship36908 Ship36909 Ship36910 Ship36913 
    Ship36914 Ship36917 Ship36918 Ship36919 Ship36920 Ship36921 
    Ship36922 Ship36923 Ship36924 Ship36925 Ship36928 Ship36929 
    Ship36930 Ship36931 Ship36933 Ship36935 Ship36936 Ship36937 
    Ship36939 Ship36942 Ship36944 Ship36946 Ship36948 Ship36949 
    Ship36950 Ship36952 Ship36953 Ship36954 Ship36955 Ship36956 
    Ship36957 Ship36958 Ship36960 Ship36962 Ship36964 Ship36965 
    Ship36966 Ship36968 Ship36970 Ship36971 Ship36972 Ship36973 
    Ship36975 Ship36976 Ship36977 Ship36980 Ship37080 Ship37180 
    Ship37280 Ship37380 Ship37580 Ship37680 Ship37702 Ship37710 
    Ship37711 Ship37716 Ship37719 Ship37733 Ship37736 Ship37742 
    Ship37744 Ship37753 Ship37765 Ship37772 Ship37780 ;

MODEL

  MAX  Profit = 
     - 2000 (OpenProduction01 + OpenProduction02 + OpenProduction03 + 
    OpenProduction04 + OpenProduction05 + OpenProduction06 + 
    OpenProduction07 + OpenProduction08 + OpenProduction09 + 
    OpenProduction10 + OpenProduction11 + OpenProduction12 + 
    OpenProduction13 + OpenProduction14 + OpenProduction16 + 
    OpenProduction17 + OpenProduction18 + OpenProduction19 + 
    OpenProduction20 + OpenProduction21 + OpenProduction22 + 
    OpenProduction23 + OpenProduction24 + OpenProduction25 + 
    OpenProduction28 + OpenProduction29 + OpenProduction30 + 
    OpenProduction31 + OpenProduction33 + OpenProduction35 + 
    OpenProduction36 + OpenProduction37 + OpenProduction39 + 
    OpenProduction41 + OpenProduction42 + OpenProduction44 + 
    OpenProduction46 + OpenProduction47 + OpenProduction48 + 
    OpenProduction49 + OpenProduction50 + OpenProduction52 + 
    OpenProduction53 + OpenProduction54 + OpenProduction55 + 
    OpenProduction56 + OpenProduction57 + OpenProduction58 + 
    OpenProduction59 + OpenProduction60 + OpenProduction62 + 
    OpenProduction63 + OpenProduction64 + OpenProduction65 + 
    OpenProduction66 + OpenProduction68 + OpenProduction69 + 
    OpenProduction70 + OpenProduction71 + OpenProduction72 + 
    OpenProduction73 + OpenProduction75 + OpenProduction76 + 
    OpenProduction77)
     - 11 (Ship10102 + Ship10103 + Ship10106) - 15 Ship10107
     - Ship10109 - 6 Ship10111 - Ship10112 - 4 Ship10113 - 2 Ship10116
     - 8 Ship10119 - 3 Ship10120 - 13 Ship10121 - 14 Ship10135
     - 8 Ship10136 - 11 Ship10137 - 5 Ship10142 - 10 Ship10148
     - 7 (Ship10152 + Ship10156 + Ship10165) - Ship10173 - 2 Ship10175
     - 6 Ship10176 - 3 Ship10504 - 10 Ship10510 - 14 Ship10511
     - 11 Ship10514 - 10 Ship10520 - 12 Ship10523 - 5 Ship10525
     - 3 Ship10547 - 7 Ship10552 - 11 Ship10553 - 15 Ship10555
     - 6 Ship10559 - 10 Ship10568 - 12 Ship10570 - 3 Ship10573
     - 14 Ship11102 - 12 Ship11103 - 2 Ship11106 - 14 Ship11108
     - 11 (Ship11109 + Ship11110) - 15 Ship11114 - 9 Ship11117
     - 4 Ship11120 - 3 Ship11121 - Ship11123 - 11 Ship11125
     - 15 Ship11128 - 11 Ship11130 - Ship11131 - 12 Ship11133
     - 11 Ship11141 - 6 Ship11142 - 2 Ship11144 - 15 Ship11146
     - 4 Ship11147 - 3 Ship11148 - 4 Ship11152 - 2 Ship11153
     - 4 Ship11156 - 6 Ship11157 - Ship11160 - 3 Ship11162
     - 10 Ship11163 - Ship11164 - 9 Ship11168
     - 6 (Ship11169 + Ship11171) - 7 Ship11172 - 4 Ship11173
     - 9 Ship11176 - 4 Ship11205 - 7 Ship11207 - 2 Ship11208
     - 15 Ship11209 - 14 Ship11218 - 6 Ship11220 - Ship11223
     - 2 Ship11225 - 10 Ship11235 - Ship11242 - 3 Ship11249
     - 15 Ship11254 - 8 Ship11256 - 7 Ship11257 - 10 Ship11260
     - 13 Ship11262 - 11 Ship11263 - 9 Ship11269 - 3 Ship11275
     - 7 Ship11418 - 3 Ship11424 - 14 Ship11425 - 11 Ship11435
     - 14 Ship11436
     - 5 (Ship11447 + Ship11449) - 12 Ship11466 - 4 Ship11468
     - 15 Ship11471 - 6 Ship11535 - 12 Ship11537 - 10 Ship11542
     - Ship11555 - 7 Ship11563 - 13 Ship11571 - 5 Ship11576
     - 11 Ship12102 - 10 Ship12105 - 14 Ship12107 - 5 Ship12113
     - 7 Ship12120 - Ship12123 - 12 Ship12125 - 10 Ship12128
     - 2 Ship12129 - 8 Ship12131 - 4 Ship12133 - 3 Ship12135
     - 6 Ship12147 - 7 Ship12148 - 9 Ship12149 - 6 Ship12150
     - 14 Ship12156 - 7 Ship12171 - 14 Ship12173 - 7 Ship12175
     - 10 Ship12403 - 9 Ship12405 - 12 Ship12406 - 7 Ship12407
     - 14 Ship12408 - 9 Ship12409 - 14 Ship12410 - 11 Ship12411
     - 6 Ship12413 - 12 Ship12417 - 11 Ship12422 - 7 Ship12423
     - Ship12429 - 11 Ship12431 - 5 Ship12433
     - 8 (Ship12435 + Ship12447) - 2 Ship12450 - 15 Ship12455
     - 4 Ship12456 - 13 Ship12465 - 9 Ship12466 - 3 Ship12468
     - 6 Ship12471 - 2 Ship12472 - 3 Ship12473 - 9 Ship12475
     - 2 Ship12477 - 9 Ship13003 - 4 Ship13007
     - 8 (Ship13008 + Ship13009) - 6 Ship13010 - Ship13013
     - 14 Ship13019 - 15 Ship13025 - 5 Ship13028 - 11 Ship13029
     - 14 Ship13031
     - 3 (Ship13035 + Ship13037) - 8 Ship13044 - 3 Ship13046
     - 8 Ship13047 - 11 Ship13048 - 12 Ship13049 - 2 Ship13050
     - 7 Ship13052 - 12 Ship13054 - 13 Ship13055 - 4 Ship13056
     - 2 Ship13058 - 9 Ship13060 - 3 Ship13062 - 12 Ship13064
     - 3 Ship13066 - 13 Ship13068 - 15 Ship13070 - 14 Ship13072
     - 13 Ship13075 - 15 Ship13076 - 11 Ship13077 - 12 Ship13402
     - 10 Ship13407 - Ship13411 - 14 Ship13439 - 7 Ship13460
     - 11 Ship14001 - 12 Ship14002 - 13 Ship14003 - 11 Ship14004
     - 14 Ship14005 - 9 Ship14006 - 5 Ship14007 - 3 Ship14008
     - 7 Ship14009 - 5 Ship14010 - 13 Ship14012 - 3 Ship14014
     - 9 Ship14016 - 4 Ship14017 - 10 Ship14018 - 8 Ship14019
     - Ship14020 - 11 Ship14021 - 10 Ship14022 - 9 Ship14023
     - 10 Ship14024 - Ship14025 - 12 Ship14028 - 4 Ship14029
     - 11 Ship14030 - 7 Ship14031 - 5 Ship14033 - 7 Ship14035
     - 11 Ship14036 - 6 Ship14037 - 14 Ship14039 - 8 Ship14041
     - 9 Ship14044 - 15 Ship14046 - 3 Ship14048 - 2 Ship14049
     - 8 Ship14050 - 3 Ship14052 - 7 Ship14053 - 12 Ship14054
     - Ship14055 - Ship14056 - 15 Ship14057 - 8 Ship14058 - 3 Ship14059
     - 15 Ship14060 - 6 Ship14062 - 13 Ship14063 - 6 Ship14066
     - 9 Ship14068 - 10 Ship14069 - 15 Ship14070 - 8 Ship14071
     - 5 Ship14073 - 13 Ship14075 - 9 Ship14076 - 11 Ship14077
     - 4 Ship14101 - 13 Ship14103 - 10 Ship14104 - 14 Ship14105
     - 7 Ship14109 - 15 Ship14112 - Ship14113 - 5 Ship14114
     - 11 Ship14117 - 12 Ship14119 - 5 Ship14136 - Ship14137
     - 9 Ship14139 - 4 Ship14144 - 15 Ship14148 - Ship14149
     - 2 Ship14150 - 4 Ship14153 - 14 Ship14155 - 9 Ship14157
     - 5 Ship14158 - 3 Ship14160
     - 15 (Ship14162 + Ship14163)
     - 10 (Ship14166 + Ship14169) - 13 Ship14171 - 7 Ship14173
     - 11 Ship14175 - 2 Ship14176 - 4 Ship14204 - 10 Ship14205
     - 8 Ship14206 - 7 Ship14218 - 9 Ship14237 - 11 Ship14249
     - 4 Ship14252 - 8 Ship14264 - 15 Ship14266 - 12 Ship14268
     - 6 Ship14301 - 12 Ship14302 - 3 Ship14303 - 14 Ship14304
     - 7 Ship14305 - 8 Ship14310 - 11 Ship14311 - 5 Ship14314
     - 12 Ship14316 - Ship14319 - 5 Ship14322 - 6 Ship14330
     - 14 Ship14337 - 7 Ship14342 - 12 Ship14344 - 5 Ship14355
     - 14 Ship14356 - 12 Ship14357 - 7 Ship14358 - 14 Ship14363
     - 8 Ship14364 - Ship14366 - 3 Ship14369 - 5 Ship14371
     - 14 (Ship14372 + Ship14373) - 5 Ship14401 - 4 Ship14404
     - 10 Ship14406 - 8 Ship14407 - 2 Ship14409 - 12 Ship14410
     - 15 Ship14414 - 11 Ship14418 - 2 Ship14419 - 7 Ship14422
     - Ship14423 - 7 Ship14428 - 3 Ship14433
     - 15 (Ship14437 + Ship14439) - Ship14446 - 7 Ship14447
     - 11 Ship14449 - 3 Ship14450 - 14 Ship14453 - 2 Ship14455
     - 13 Ship14457 - 4 Ship14458 - 6 Ship14459 - 9 Ship14465
     - 12 Ship14472 - 14 Ship14473 - 6 Ship14475 - Ship14508
     - 14 Ship14511 - 15 Ship14512 - 4 Ship14514 - 7 Ship14516
     - 9 Ship14518 - 4 Ship14521 - 8 Ship14529 - 10 Ship14530
     - 8 Ship14544 - 15 Ship14547 - 9 Ship14548 - 12 Ship14549
     - Ship14554 - 10 Ship14556 - 3 Ship14557 - 5 Ship14558
     - 4 Ship14559
     - 9 (Ship14562 + Ship14565) - 14 Ship14569 - 4 Ship14570
     - 6 Ship14572 - 5 Ship14576 - 2 Ship14577 - 13 Ship14703
     - 4 Ship14704 - 3 Ship14706 - 4 Ship14709
     - 12 (Ship14713 + Ship14716) - 7 Ship14717 - 2 Ship14718
     - 11 Ship14719 - 5 Ship14720 - 11 Ship14723 - 10 Ship14725
     - 3 Ship14733 - 8 Ship14737
     - 7 (Ship14739 + Ship14742) - 4 Ship14746 - 15 Ship14749
     - 9 (Ship14750 + Ship14755) - 2 Ship14757 - 15 Ship14758
     - Ship14760 - 11 Ship14762
     - 12 (Ship14763 + Ship14764) - 4 Ship14765 - 2 Ship14768
     - 13 Ship14769 - 8 Ship14771 - 15 Ship14772 - 7 Ship14777
     - 6 Ship15010 - 5 Ship15014 - 9 Ship15017 - 12 Ship15035
     - 8 Ship15036 - 13 Ship15037 - 6 Ship15054 - 12 Ship15057
     - 7 Ship15062 - 6 Ship15064 - 10 Ship15066 - 6 Ship15068
     - 11 Ship15072 - 5 Ship15073 - Ship15075 - 12 Ship16013
     - 11 Ship16014 - 2 Ship16019 - 7 Ship16020 - 3 Ship16024
     - 15 (Ship16031 + Ship16037) - 3 Ship16039 - 5 Ship16048
     - 9 Ship16053 - 6 Ship16058 - 15 Ship16062 - 14 Ship16072
     - 10 (Ship16103 + Ship16104 + Ship16105 + Ship16106)
     - 14 Ship16108 - 4 Ship16111 - 15 Ship16114 - 10 Ship16119
     - 12 Ship16122 - 14 Ship16124 - 15 Ship16128 - 5 Ship16129
     - 2 Ship16135 - 7 Ship16136
     - 10 (Ship16142 + Ship16144) - 8 Ship16149
     - 14 (Ship16153 + Ship16154) - 15 Ship16155 - 8 Ship16156
     - 7 Ship16157 - 12 Ship16159 - 6 Ship16164 - 14 Ship16166
     - 8 Ship16168 - Ship16171 - 3 Ship16177 - 12 Ship16701
     - 4 Ship16709 - 14 Ship16729 - 8 Ship16753
     - 7 (Ship16756 + Ship16773) - 4 Ship17901 - 14 Ship17902
     - 6 Ship17903 - 15 Ship17904 - 8 Ship17905 - 2 Ship17907
     - 6 Ship17908 - 3 Ship17909 - 11 Ship17910 - 13 Ship17911
     - 10 Ship17913 - 7 Ship17914 - 14 Ship17917 - 15 Ship17918
     - 7 Ship17919 - 9 Ship17920 - 11 Ship17921 - 10 Ship17922
     - Ship17923 - 6 Ship17924 - 2 Ship17925 - 10 Ship17928 - Ship17929
     - 14 Ship17930 - 15 Ship17931 - 3 Ship17933 - 2 Ship17935
     - 6 Ship17936 - 15 Ship17937 - 11 Ship17939 - 5 Ship17941
     - 13 Ship17942 - 12 Ship17944 - 9 Ship17946 - 14 Ship17947
     - 7 Ship17949 - 14 Ship17950 - 13 Ship17952 - 15 Ship17953
     - 3 Ship17954 - 10 Ship17955 - 9 Ship17958 - Ship17959
     - 9 Ship17962 - 13 Ship17963 - 12 Ship17964 - 2 Ship17965
     - 4 Ship17968 - Ship17969 - 8 Ship17970 - 10 Ship17973
     - 8 (Ship17975 + Ship17976) - 14 Ship20203 - 6 Ship20206
     - 8 Ship20208 - 15 Ship20209 - 12 Ship20210 - 8 Ship20212
     - 6 Ship20214 - Ship20217 - 2 Ship20218 - Ship20219
     - 9 (Ship20220 + Ship20222) - Ship20223 - 4 Ship20224
     - 8 Ship20225 - 6 Ship20228 - 4 Ship20229 - 11 Ship20231
     - 13 Ship20235 - 4 Ship20237 - 2 Ship20239 - 11 Ship20241
     - 13 Ship20242 - 8 Ship20244 - 7 Ship20246 - 15 Ship20248
     - Ship20252 - 12 Ship20254 - 5 Ship20257 - 6 Ship20258
     - 7 Ship20259
     - 13 (Ship20263 + Ship20264) - 7 Ship20265 - 10 Ship20266
     - 2 Ship20268
     - 8 (Ship20269 + Ship20270) - Ship20271 - 15 Ship20275
     - 14 Ship20276 - 6 Ship21602 - Ship21603 - 7 Ship21605
     - 10 Ship21606 - 6 Ship21607 - 3 Ship21610 - 7 Ship21611
     - 11 Ship21613 - 2 Ship21621 - 7 Ship21622 - 12 Ship21624
     - 6 Ship21625 - 8 Ship21628 - 7 Ship21633 - 8 Ship21635
     - 7 Ship21637 - 15 Ship21641 - Ship21648 - 13 Ship21655
     - 9 Ship21657 - 14 Ship21662 - 7 Ship21664 - 11 Ship21665
     - 5 Ship21666 - 4 Ship21668 - 5 Ship21669 - 15 Ship21670
     - 5 Ship21672
     - 4 (Ship21675 + Ship22307) - Ship22308 - 11 Ship22309
     - 13 Ship22314 - 4 Ship22317 - 5 Ship22318 - 10 Ship22329
     - Ship22330
     - 11 (Ship22331 + Ship22337) - 9 Ship22341 - 8 Ship22342
     - 6 Ship22348 - 10 Ship22350 - 4 Ship22352 - 13 Ship22362
     - Ship22364 - 2 Ship22368 - 3 Ship22372 - 13 Ship22373
     - 3 Ship22375 - 14 Ship22601 - 2 Ship22602
     - 4 (Ship22605 + Ship22609) - Ship22610 - 4 Ship22618
     - 12 Ship22620 - 14 Ship22623 - 13 Ship22624 - Ship22635
     - 4 (Ship22637 + Ship22642) - 7 Ship22647 - 8 Ship22654
     - Ship22656 - 14 Ship22664 - 15 Ship22673 - 11 Ship22703
     - 5 Ship22705 - 10 Ship22706 - 6 Ship22707 - 14 Ship22709
     - 2 Ship22716 - 8 Ship22720 - 12 Ship22722 - 5 Ship22723
     - 10 Ship22724 - 7 Ship22731 - 5 Ship22733 - 7 Ship22735
     - 5 Ship22741 - 7 Ship22742 - 8 Ship22744 - 12 Ship22746
     - 11 Ship22749 - 12 Ship22750 - 5 Ship22753 - 13 Ship22754
     - 10 Ship22758 - 2 Ship22759 - 8 Ship22760 - 9 Ship22762
     - 14 Ship22763 - 11 Ship22765 - 5 Ship22769 - 7 Ship22770
     - 13 Ship22773 - 9 Ship22777 - 13 Ship23101 - 10 Ship23103
     - 9 Ship23104 - 8 Ship23105 - 7 Ship23107 - 14 Ship23109
     - 4 Ship23110 - 15 Ship23112 - 11 Ship23113 - 10 Ship23116
     - Ship23118 - 4 Ship23119 - 10 Ship23120 - 4 Ship23125
     - 14 Ship23128 - 3 Ship23129 - 14 Ship23133 - 3 Ship23135
     - 6 Ship23136 - 15 Ship23137 - 3 Ship23139 - 4 Ship23141
     - 10 Ship23144 - 6 Ship23146 - 9 Ship23147 - 8 Ship23149
     - 10 Ship23150 - 4 Ship23152 - 10 Ship23153 - 8 Ship23154
     - 5 Ship23155 - 8 Ship23159 - 6 Ship23162
     - 3 (Ship23164 + Ship23165) - 13 Ship23166 - 12 Ship23168
     - 14 Ship23170 - 13 Ship23171 - 4 Ship23172 - 7 Ship23173
     - 6 Ship23175 - 9 Ship23176 - 12 Ship23177 - 8 Ship23201
     - Ship23203 - 6 Ship23205 - 14 Ship23206 - 6 Ship23208
     - 15 Ship23210 - 7 Ship23213 - 14 Ship23216 - Ship23217
     - 2 Ship23218 - Ship23219 - 6 Ship23222 - 13 Ship23225
     - 15 Ship23229 - 11 Ship23235 - 2 Ship23236 - 4 Ship23244
     - 2 Ship23246 - 3 Ship23248 - 5 Ship23249 - 2 Ship23250
     - 5 Ship23252 - 3 Ship23253 - 4 Ship23257 - 15 Ship23266
     - 7 Ship23269 - 5 Ship23270 - 6 Ship23271 - 4 Ship23272
     - 6 Ship23275 - 12 Ship23402 - 10 Ship23407 - Ship23411
     - 14 Ship23439 - 7 Ship23460 - 2 Ship23801 - Ship23804
     - 7 Ship23805 - 3 Ship23808 - 7 Ship23811 - 13 Ship23812
     - 7 Ship23814 - 2 Ship23816 - Ship23818 - 13 Ship23820
     - 12 Ship23823 - Ship23828 - 2 Ship23829 - 14 Ship23831
     - 12 Ship23836 - 11 Ship23841 - 15 Ship23842
     - 2 (Ship23844 + Ship23849) - 5 Ship23855 - 8 Ship23857
     - 15 Ship23859 - 9 Ship23862 - 2 Ship23863 - 5 Ship23864
     - 7 Ship23865 - 2 Ship23866 - 7 Ship23868 - 5 Ship23869
     - 13 Ship23871 - 11 Ship23872 - 12 Ship23873 - 10 Ship23876
     - Ship24508 - 14 Ship24511 - 15 Ship24512 - 4 Ship24514
     - 7 Ship24516 - 9 Ship24518 - 4 Ship24521 - 8 Ship24529
     - 10 Ship24530 - 8 Ship24544 - 15 Ship24547 - 9 Ship24548
     - 12 Ship24549 - Ship24554 - 10 Ship24556 - 3 Ship24557
     - 5 Ship24558 - 4 Ship24559
     - 9 (Ship24562 + Ship24565) - 14 Ship24569 - 4 Ship24570
     - 6 Ship24572 - 5 Ship24576 - 2 Ship24577 - 7 Ship25101
     - 3 Ship25103 - 7 Ship25104 - 2 Ship25105 - 5 Ship25109
     - 12 Ship25111 - 14 Ship25112 - Ship25116 - 5 Ship25117
     - 13 (Ship25119 + Ship25120) - 6 Ship25121 - 11 Ship25123
     - 2 Ship25124 - 13 Ship25125 - 9 Ship25128 - 6 Ship25129
     - 13 Ship25130 - 3 Ship25133 - 14 Ship25135 - 7 Ship25136
     - 15 Ship25137 - 14 Ship25139 - 8 Ship25141 - 2 Ship25144
     - 10 Ship25147 - 14 Ship25148 - 2 Ship25149 - 11 Ship25150
     - 8 Ship25152 - 12 Ship25153 - 13 Ship25154 - 9 Ship25155
     - 2 Ship25156 - 15 Ship25157 - 10 Ship25158 - Ship25159
     - 9 Ship25160 - 11 Ship25162 - 7 Ship25163 - 2 Ship25164
     - 15 Ship25166 - Ship25170 - 12 Ship25171 - 11 Ship25173
     - 7 Ship25175 - 13 Ship25177 - 2 Ship25502 - 7 Ship25503
     - 6 Ship25504 - 2 Ship25506 - 10 Ship25508 - 11 Ship25509
     - 12 Ship25512 - Ship25514 - 9 Ship25519 - 2 Ship25525
     - 8 Ship25529 - Ship25536
     - 7 (Ship25537 + Ship25539) - 14 Ship25542 - 8 Ship25548
     - 7 Ship25549 - 11 Ship25550 - 5 Ship25554 - 6 Ship25556
     - 4 Ship25557 - 2 Ship25560 - 11 Ship25563
     - 8 (Ship25573 + Ship25575) - 15 Ship25576 - 9 Ship25577
     - 8 Ship25903 - 9 Ship25904 - 3 Ship25909 - 8 Ship25911
     - 11 Ship25918 - 6 Ship25922 - 8 Ship25942 - 10 Ship25946
     - 15 Ship25948 - 6 Ship25955 - 8 Ship25956 - Ship25957
     - 8 Ship25972 - 5 Ship25976 - Ship25977
     - 10 (Ship26103 + Ship26104 + Ship26105 + Ship26106)
     - 14 Ship26108 - 4 Ship26111 - 15 Ship26114 - 10 Ship26119
     - 12 Ship26122 - 14 Ship26124 - 15 Ship26128 - 5 Ship26129
     - 2 Ship26135 - 7 Ship26136
     - 10 (Ship26142 + Ship26144) - 8 Ship26149
     - 14 (Ship26153 + Ship26154) - 15 Ship26155 - 8 Ship26156
     - 7 Ship26157 - 12 Ship26159 - 6 Ship26164 - 14 Ship26166
     - 8 Ship26168 - Ship26171 - 3 Ship26177 - 7 Ship26301
     - 11 Ship26304 - Ship26305 - 3 Ship26306 - 7 Ship26307
     - 9 Ship26308
     - 7 (Ship26309 + Ship26310) - 10 Ship26313 - 3 Ship26314
     - 5 Ship26316 - 4 Ship26317 - 2 Ship26318 - 3 Ship26319
     - 5 Ship26320 - 12 Ship26322 - 5 Ship26323 - 8 Ship26325
     - 9 Ship26328 - 2 Ship26329 - 9 Ship26331
     - 4 (Ship26333 + Ship26335) - Ship26336
     - 7 (Ship26337 + Ship26339) - 15 Ship26342 - 12 Ship26344
     - 6 Ship26346 - 11 Ship26348 - 6 Ship26349 - 8 Ship26352
     - 7 Ship26354 - Ship26356 - Ship26357 - 4 Ship26358 - 2 Ship26359
     - 8 Ship26360 - 5 Ship26362
     - 2 (Ship26364 + Ship26365) - 14 Ship26369 - 2 Ship26371
     - 4 Ship26372 - 7 Ship26373 - 13 Ship26375 - 12 Ship26377
     - 6 (Ship26901 + Ship26903) - 10 Ship26904 - Ship26906
     - 11 Ship26907 - 7 Ship26908 - 2 Ship26909 - Ship26910
     - 14 Ship26913 - 6 Ship26914 - 11 Ship26917 - 5 Ship26918
     - 14 Ship26919 - 4 Ship26920 - Ship26921 - 9 Ship26922
     - 4 Ship26923 - 9 Ship26924 - 7 Ship26925 - 5 Ship26928
     - 8 Ship26929 - Ship26930 - 11 Ship26931 - 6 Ship26933
     - 15 Ship26935 - 14 Ship26936 - 12 Ship26937 - 5 Ship26939
     - 15 Ship26942 - 14 Ship26944 - 2 Ship26946 - 13 Ship26948
     - 15 Ship26949 - 11 Ship26950 - 13 Ship26952 - 7 Ship26953
     - 11 Ship26954 - 3 Ship26955 - 14 Ship26956 - Ship26957
     - 8 Ship26958 - 10 Ship26960 - 12 Ship26962 - 5 Ship26964
     - 9 Ship26965 - 3 Ship26966 - 6 Ship26968 - 13 Ship26970
     - 6 Ship26971 - 15 Ship26972 - 9 Ship26973 - 10 Ship26975
     - 15 Ship26976 - 8 Ship26977 - 9 Ship27401 - 8 Ship27402
     - 6 Ship27404 - 5 Ship27405 - 2 Ship27407 - 5 Ship27408
     - Ship27409 - 15 Ship27414 - 2 Ship27416 - 5 Ship27420
     - 4 Ship27421 - 5 Ship27423 - 3 Ship27424 - 7 Ship27429
     - 6 Ship27436 - 13 Ship27437 - 11 Ship27439 - Ship27444
     - 10 Ship27448 - Ship27449 - 2 Ship27450 - 6 Ship27452
     - 15 Ship27453 - 3 Ship27454 - Ship27455 - 6 Ship27456
     - 7 Ship27457 - 2 Ship27458 - 15 Ship27460 - 10 Ship27462
     - 7 Ship27463 - 10 Ship27464 - Ship27465 - 3 Ship27466
     - 9 Ship27468 - 13 Ship27471 - Ship27472 - 9 Ship27473
     - 15 Ship27476 - 12 Ship27702 - Ship27710 - 7 Ship27711
     - 3 Ship27716 - 9 Ship27719 - 13 Ship27733 - 8 Ship27736
     - 14 Ship27742 - 3 Ship27744 - 12 Ship27753 - 7 Ship27765
     - 6 Ship27772 - 12 Ship27801 - 4 Ship27805 - 10 Ship27811
     - 6 Ship27821 - 5 Ship27829 - 8 Ship27830 - 6 Ship27836
     - 5 (Ship27837 + Ship27847) - 2 Ship27850 - 3 Ship27852
     - 13 (Ship27857 + Ship27864) - 14 Ship27873 - 4 Ship27901
     - 14 Ship27902 - 6 Ship27903 - 15 Ship27904 - 8 Ship27905
     - 2 Ship27907 - 6 Ship27908 - 3 Ship27909 - 11 Ship27910
     - 13 Ship27911 - 10 Ship27913 - 7 Ship27914 - 14 Ship27917
     - 15 Ship27918 - 7 Ship27919 - 9 Ship27920 - 11 Ship27921
     - 10 Ship27922 - Ship27923 - 6 Ship27924 - 2 Ship27925
     - 10 Ship27928 - Ship27929 - 14 Ship27930 - 15 Ship27931
     - 3 Ship27933 - 2 Ship27935 - 6 Ship27936 - 15 Ship27937
     - 11 Ship27939 - 5 Ship27941 - 13 Ship27942 - 12 Ship27944
     - 9 Ship27946 - 14 Ship27947 - 7 Ship27949 - 14 Ship27950
     - 13 Ship27952 - 15 Ship27953 - 3 Ship27954 - 10 Ship27955
     - 9 Ship27958 - Ship27959 - 9 Ship27962 - 13 Ship27963
     - 12 Ship27964 - 2 Ship27965 - 4 Ship27968 - Ship27969
     - 8 Ship27970 - 10 Ship27973
     - 8 (Ship27975 + Ship27976)
     - 11 (Ship30102 + Ship30103 + Ship30106) - 15 Ship30107
     - Ship30109 - 6 Ship30111 - Ship30112 - 4 Ship30113 - 2 Ship30116
     - 8 Ship30119 - 3 Ship30120 - 13 Ship30121 - 14 Ship30135
     - 8 Ship30136 - 11 Ship30137 - 5 Ship30142 - 10 Ship30148
     - 7 (Ship30152 + Ship30156 + Ship30165) - Ship30173 - 2 Ship30175
     - 6 Ship30176 - 14 Ship30203 - 6 Ship30206 - 8 Ship30208
     - 15 Ship30209 - 12 Ship30210 - 8 Ship30212 - 6 Ship30214
     - Ship30217 - 2 Ship30218 - Ship30219
     - 9 (Ship30220 + Ship30222) - Ship30223 - 4 Ship30224
     - 8 Ship30225 - 6 Ship30228 - 4 Ship30229 - 11 Ship30231
     - 13 Ship30235 - 4 Ship30237 - 2 Ship30239 - 11 Ship30241
     - 13 Ship30242 - 8 Ship30244 - 7 Ship30246 - 15 Ship30248
     - Ship30252 - 12 Ship30254 - 5 Ship30257 - 6 Ship30258
     - 7 Ship30259
     - 13 (Ship30263 + Ship30264) - 7 Ship30265 - 10 Ship30266
     - 2 Ship30268
     - 8 (Ship30269 + Ship30270) - Ship30271 - 15 Ship30275
     - 14 Ship30276 + 5 Ship30280 + 4 Ship30380 + 10 Ship30480
     - 3 Ship30504 - 10 Ship30510 - 14 Ship30511 - 11 Ship30514
     - 10 Ship30520 - 12 Ship30523 - 5 Ship30525 - 3 Ship30547
     - 7 Ship30552 - 11 Ship30553 - 15 Ship30555 - 6 Ship30559
     - 10 Ship30568 - 12 Ship30570 - 3 Ship30573 - 2 Ship30580
     + 11 Ship30680
     + 5 (Ship30780 + Ship30880) + 7 Ship31080 - 14 Ship31102
     - 12 Ship31103 - 2 Ship31106 - 14 Ship31108
     - 11 (Ship31109 + Ship31110) - 15 Ship31114 - 9 Ship31117
     - 4 Ship31120 - 3 Ship31121 - Ship31123 - 11 Ship31125
     - 15 Ship31128 - 11 Ship31130 - Ship31131 - 12 Ship31133
     - 11 Ship31141 - 6 Ship31142 - 2 Ship31144 - 15 Ship31146
     - 4 Ship31147 - 3 Ship31148 - 4 Ship31152 - 2 Ship31153
     - 4 Ship31156 - 6 Ship31157 - Ship31160 - 3 Ship31162
     - 10 Ship31163 - Ship31164 - 9 Ship31168
     - 6 (Ship31169 + Ship31171) - 7 Ship31172 - 4 Ship31173
     - 9 Ship31176 + 9 Ship31180 - 4 Ship31205 - 7 Ship31207
     - 2 Ship31208 - 15 Ship31209 - 14 Ship31218 - 6 Ship31220
     - Ship31223 - 2 Ship31225 - 10 Ship31235 - Ship31242 - 3 Ship31249
     - 15 Ship31254 - 8 Ship31256 - 7 Ship31257 - 10 Ship31260
     - 13 Ship31262 - 11 Ship31263 - 9 Ship31269 - 3 Ship31275
     - Ship31280 + 8 Ship31380 - 7 Ship31418 - 3 Ship31424
     - 14 Ship31425 - 11 Ship31435 - 14 Ship31436
     - 5 (Ship31447 + Ship31449) - 12 Ship31466 - 4 Ship31468
     - 15 Ship31471 + 11 Ship31480 - 6 Ship31602 - Ship31603
     - 7 Ship31605 - 10 Ship31606 - 6 Ship31607 - 3 Ship31610
     - 7 Ship31611 - 11 Ship31613 - 2 Ship31621 - 7 Ship31622
     - 12 Ship31624 - 6 Ship31625 - 8 Ship31628 - 7 Ship31633
     - 8 Ship31635 - 7 Ship31637 - 15 Ship31641 - Ship31648
     - 13 Ship31655 - 9 Ship31657 - 14 Ship31662 - 7 Ship31664
     - 11 Ship31665 - 5 Ship31666 - 4 Ship31668 - 5 Ship31669
     - 15 Ship31670 - 5 Ship31672 - 4 Ship31675 + 3 Ship31680
     + 5 (Ship31780 + Ship31880) + 10 Ship31980 - 11 Ship32102
     - 10 Ship32105 - 14 Ship32107 - 5 Ship32113 - 7 Ship32120
     - Ship32123 - 12 Ship32125 - 10 Ship32128 - 2 Ship32129
     - 8 Ship32131 - 4 Ship32133 - 3 Ship32135 - 6 Ship32147
     - 7 Ship32148 - 9 Ship32149 - 6 Ship32150 - 14 Ship32156
     - 7 Ship32171 - 14 Ship32173 - 7 Ship32175 + 4 Ship32180
     + 5 Ship32280 - 4 Ship32307 - Ship32308 - 11 Ship32309
     - 13 Ship32314 - 4 Ship32317 - 5 Ship32318 - 10 Ship32329
     - Ship32330
     - 11 (Ship32331 + Ship32337) - 9 Ship32341 - 8 Ship32342
     - 6 Ship32348 - 10 Ship32350 - 4 Ship32352 - 13 Ship32362
     - Ship32364 - 2 Ship32368 - 3 Ship32372 - 13 Ship32373
     - 3 Ship32375 + 11 Ship32380 - 10 Ship32403 - 9 Ship32405
     - 12 Ship32406 - 7 Ship32407 - 14 Ship32408 - 9 Ship32409
     - 14 Ship32410 - 11 Ship32411 - 6 Ship32413 - 12 Ship32417
     - 11 Ship32422 - 7 Ship32423 - Ship32429 - 11 Ship32431
     - 5 Ship32433
     - 8 (Ship32435 + Ship32447) - 2 Ship32450 - 15 Ship32455
     - 4 Ship32456 - 13 Ship32465 - 9 Ship32466 - 3 Ship32468
     - 6 Ship32471 - 2 Ship32472 - 3 Ship32473 - 9 Ship32475
     - 2 Ship32477 + 6 Ship32480 + 9 Ship32580 + 2 Ship32880
     + 3 Ship32980 - 9 Ship33003 - 4 Ship33007
     - 8 (Ship33008 + Ship33009) - 6 Ship33010 - Ship33013
     - 14 Ship33019 - 15 Ship33025 - 5 Ship33028 - 11 Ship33029
     - 14 Ship33031
     - 3 (Ship33035 + Ship33037) - 8 Ship33044 - 3 Ship33046
     - 8 Ship33047 - 11 Ship33048 - 12 Ship33049 - 2 Ship33050
     - 7 Ship33052 - 12 Ship33054 - 13 Ship33055 - 4 Ship33056
     - 2 Ship33058 - 9 Ship33060 - 3 Ship33062 - 12 Ship33064
     - 3 Ship33066 - 13 Ship33068 - 15 Ship33070 - 14 Ship33072
     - 13 Ship33075 - 15 Ship33076 - 11 Ship33077 + 8 Ship33080
     - 13 Ship33101 - 10 Ship33103 - 9 Ship33104 - 8 Ship33105
     - 7 Ship33107 - 14 Ship33109 - 4 Ship33110 - 15 Ship33112
     - 11 Ship33113 - 10 Ship33116 - Ship33118 - 4 Ship33119
     - 10 Ship33120 - 4 Ship33125 - 14 Ship33128 - 3 Ship33129
     - 14 Ship33133 - 3 Ship33135 - 6 Ship33136 - 15 Ship33137
     - 3 Ship33139 - 4 Ship33141 - 10 Ship33144 - 6 Ship33146
     - 9 Ship33147 - 8 Ship33149 - 10 Ship33150 - 4 Ship33152
     - 10 Ship33153 - 8 Ship33154 - 5 Ship33155 - 8 Ship33159
     - 6 Ship33162
     - 3 (Ship33164 + Ship33165) - 13 Ship33166 - 12 Ship33168
     - 14 Ship33170 - 13 Ship33171 - 4 Ship33172 - 7 Ship33173
     - 6 Ship33175 - 9 Ship33176 - 12 Ship33177 - 2 Ship33180
     + 5 Ship33380 + 11 Ship33580 + 9 Ship33680 + 8 Ship33780
     + 11 Ship33980 - 4 Ship34101 - 13 Ship34103 - 10 Ship34104
     - 14 Ship34105 - 7 Ship34109 - 15 Ship34112 - Ship34113
     - 5 Ship34114 - 11 Ship34117 - 12 Ship34119 - 5 Ship34136
     - Ship34137 - 9 Ship34139 - 4 Ship34144 - 15 Ship34148 - Ship34149
     - 2 Ship34150 - 4 Ship34153 - 14 Ship34155 - 9 Ship34157
     - 5 Ship34158 - 3 Ship34160
     - 15 (Ship34162 + Ship34163)
     - 10 (Ship34166 + Ship34169) - 13 Ship34171 - 7 Ship34173
     - 11 Ship34175 - 2 Ship34176 + 2 Ship34180 - 4 Ship34204
     - 10 Ship34205 - 8 Ship34206 - 7 Ship34218 - 9 Ship34237
     - 11 Ship34249 - 4 Ship34252 - 8 Ship34264 - 15 Ship34266
     - 12 Ship34268 + 11 Ship34280 - 5 Ship34401 - 4 Ship34404
     - 10 Ship34406 - 8 Ship34407 - 2 Ship34409 - 12 Ship34410
     - 15 Ship34414 - 11 Ship34418 - 2 Ship34419 - 7 Ship34422
     - Ship34423 - 7 Ship34428 - 3 Ship34433
     - 15 (Ship34437 + Ship34439) - Ship34446 - 7 Ship34447
     - 11 Ship34449 - 3 Ship34450 - 14 Ship34453 - 2 Ship34455
     - 13 Ship34457 - 4 Ship34458 - 6 Ship34459 - 9 Ship34465
     - 12 Ship34472 - 14 Ship34473 - 6 Ship34475 + 5 Ship34680
     - 13 Ship34703 - 4 Ship34704 - 3 Ship34706 - 4 Ship34709
     - 12 (Ship34713 + Ship34716) - 7 Ship34717 - 2 Ship34718
     - 11 Ship34719 - 5 Ship34720 - 11 Ship34723 - 10 Ship34725
     - 3 Ship34733 - 8 Ship34737
     - 7 (Ship34739 + Ship34742) - 4 Ship34746 - 15 Ship34749
     - 9 (Ship34750 + Ship34755) - 2 Ship34757 - 15 Ship34758
     - Ship34760 - 11 Ship34762
     - 12 (Ship34763 + Ship34764) - 4 Ship34765 - 2 Ship34768
     - 13 Ship34769 - 8 Ship34771 - 15 Ship34772 - 7 Ship34777
     + Ship34780 + 2 Ship34880 - 6 Ship35010 - 5 Ship35014
     - 9 Ship35017 - 12 Ship35035 - 8 Ship35036 - 13 Ship35037
     - 6 Ship35054 - 12 Ship35057 - 7 Ship35062 - 6 Ship35064
     - 10 Ship35066 - 6 Ship35068 - 11 Ship35072 - 5 Ship35073
     - Ship35075 + 6 Ship35080 + 8 Ship35380 + 7 Ship35480
     - 2 Ship35502 - 7 Ship35503 - 6 Ship35504 - 2 Ship35506
     - 10 Ship35508 - 11 Ship35509 - 12 Ship35512 - Ship35514
     - 9 Ship35519 - 2 Ship35525 - 8 Ship35529 - Ship35536
     - 7 (Ship35537 + Ship35539) - 14 Ship35542 - 8 Ship35548
     - 7 Ship35549 - 11 Ship35550 - 5 Ship35554 - 6 Ship35556
     - 4 Ship35557 - 2 Ship35560 - 11 Ship35563
     - 8 (Ship35573 + Ship35575) - 15 Ship35576 - 9 Ship35577
     - 2 Ship35580 + 9 Ship35880 - 8 Ship35903 - 9 Ship35904
     - 3 Ship35909 - 8 Ship35911 - 11 Ship35918 - 6 Ship35922
     - 8 Ship35942 - 10 Ship35946 - 15 Ship35948 - 6 Ship35955
     - 8 Ship35956 - Ship35957 - 8 Ship35972 - 5 Ship35976 - Ship35977
     - 2 Ship35980 - 12 Ship36013 - 11 Ship36014 - 2 Ship36019
     - 7 Ship36020 - 3 Ship36024
     - 15 (Ship36031 + Ship36037) - 3 Ship36039 - 5 Ship36048
     - 9 Ship36053 - 6 Ship36058 - 15 Ship36062 - 14 Ship36072
     + 11 Ship36080 + 4 Ship36280 - 7 Ship36301 - 11 Ship36304
     - Ship36305 - 3 Ship36306 - 7 Ship36307 - 9 Ship36308
     - 7 (Ship36309 + Ship36310) - 10 Ship36313 - 3 Ship36314
     - 5 Ship36316 - 4 Ship36317 - 2 Ship36318 - 3 Ship36319
     - 5 Ship36320 - 12 Ship36322 - 5 Ship36323 - 8 Ship36325
     - 9 Ship36328 - 2 Ship36329 - 9 Ship36331
     - 4 (Ship36333 + Ship36335) - Ship36336
     - 7 (Ship36337 + Ship36339) - 15 Ship36342 - 12 Ship36344
     - 6 Ship36346 - 11 Ship36348 - 6 Ship36349 - 8 Ship36352
     - 7 Ship36354 - Ship36356 - Ship36357 - 4 Ship36358 - 2 Ship36359
     - 8 Ship36360 - 5 Ship36362
     - 2 (Ship36364 + Ship36365) - 14 Ship36369 - 2 Ship36371
     - 4 Ship36372 - 7 Ship36373 - 13 Ship36375 - 12 Ship36377
     + 12 Ship36380
     + 7 (Ship36480 + Ship36580) + 4 Ship36680 - 2 Ship36880
     - 6 (Ship36901 + Ship36903) - 10 Ship36904 - Ship36906
     - 11 Ship36907 - 7 Ship36908 - 2 Ship36909 - Ship36910
     - 14 Ship36913 - 6 Ship36914 - 11 Ship36917 - 5 Ship36918
     - 14 Ship36919 - 4 Ship36920 - Ship36921 - 9 Ship36922
     - 4 Ship36923 - 9 Ship36924 - 7 Ship36925 - 5 Ship36928
     - 8 Ship36929 - Ship36930 - 11 Ship36931 - 6 Ship36933
     - 15 Ship36935 - 14 Ship36936 - 12 Ship36937 - 5 Ship36939
     - 15 Ship36942 - 14 Ship36944 - 2 Ship36946 - 13 Ship36948
     - 15 Ship36949 - 11 Ship36950 - 13 Ship36952 - 7 Ship36953
     - 11 Ship36954 - 3 Ship36955 - 14 Ship36956 - Ship36957
     - 8 Ship36958 - 10 Ship36960 - 12 Ship36962 - 5 Ship36964
     - 9 Ship36965 - 3 Ship36966 - 6 Ship36968 - 13 Ship36970
     - 6 Ship36971 - 15 Ship36972 - 9 Ship36973 - 10 Ship36975
     - 15 Ship36976 - 8 Ship36977
     - 2 (Ship36980 + Ship37080)
     + 12 (Ship37280 + Ship37380 + Ship37580) + 2 Ship37680
     - 12 Ship37702 - Ship37710 - 7 Ship37711 - 3 Ship37716
     - 9 Ship37719 - 13 Ship37733 - 8 Ship37736 - 14 Ship37742
     - 3 Ship37744 - 12 Ship37753 - 7 Ship37765 - 6 Ship37772
     + 5 Ship37780 ;

SUBJECT TO

  BOMConstr011: 2 Ship30180  =  Ship10101 + Ship14001 + Ship14101
     + Ship14301 + Ship14401 + Ship16701 + Ship17901 ;
  BOMConstr012: Ship30180  =  Ship22601 + Ship23101 + Ship23201
     + Ship23801 + Ship25101 + Ship26301 + Ship26901 + Ship27401
     + Ship27801 + Ship27901 ;
  BOMConstr021: 2 Ship30280  =  Ship10102 + Ship11102 + Ship12102
     + Ship13402 + Ship14002 + Ship14302 + Ship17902 ;
  BOMConstr022: Ship30280  =  Ship20202 + Ship21602 + Ship22602
     + Ship23402 + Ship25502 + Ship27402 + Ship27702 + Ship27902 ;
  BOMConstr031: 2 Ship30380  =  Ship10103 + Ship11103 + Ship12403
     + Ship13003 + Ship14003 + Ship14103 + Ship14303 + Ship14703
     + Ship16103 + Ship17903 ;
  BOMConstr032: Ship30380  =  Ship20203 + Ship21603 + Ship22703
     + Ship23103 + Ship23203 + Ship25103 + Ship25503 + Ship25903
     + Ship26103 + Ship26903 + Ship27903 ;
  BOMConstr041: 2 Ship30480  =  Ship10504 + Ship14004 + Ship14104
     + Ship14204 + Ship14304 + Ship14404 + Ship14704 + Ship16104
     + Ship17904 ;
  BOMConstr042: Ship30480  =  Ship23104 + Ship23804 + Ship25104
     + Ship25504 + Ship25904 + Ship26104 + Ship26304 + Ship26904
     + Ship27404 + Ship27904 ;
  BOMConstr051: 2 Ship30580  =  Ship10505 + Ship11205 + Ship12105
     + Ship12405 + Ship14005 + Ship14105 + Ship14205 + Ship14305
     + Ship16105 + Ship17905 ;
  BOMConstr052: Ship30580  =  Ship21605 + Ship22605 + Ship22705
     + Ship23105 + Ship23205 + Ship23805 + Ship25105 + Ship26105
     + Ship26305 + Ship27405 + Ship27805 + Ship27905 ;
  BOMConstr061: 2 Ship30680  =  Ship10106 + Ship11106 + Ship12406
     + Ship14006 + Ship14206 + Ship14406 + Ship14706 + Ship16106 ;
  BOMConstr062: Ship30680  =  Ship20206 + Ship21606 + Ship22706
     + Ship23206 + Ship25506 + Ship26106 + Ship26306 + Ship26906 ;
  BOMConstr071: 2 Ship30780  =  Ship10107 + Ship11207 + Ship12107
     + Ship12407 + Ship13007 + Ship13407 + Ship14007 + Ship14407
     + Ship17907 ;
  BOMConstr072: Ship30780  =  Ship21607 + Ship22307 + Ship22707
     + Ship23107 + Ship23407 + Ship26307 + Ship26907 + Ship27407
     + Ship27907 ;
  BOMConstr081: 2 Ship30880  =  Ship11108 + Ship11208 + Ship12408
     + Ship13008 + Ship14008 + Ship14508 + Ship16108 + Ship17908 ;
  BOMConstr082: Ship30880  =  Ship20208 + Ship22308 + Ship23208
     + Ship23808 + Ship24508 + Ship25508 + Ship26108 + Ship26308
     + Ship26908 + Ship27408 + Ship27908 ;
  BOMConstr091: 2 Ship30980  =  Ship10109 + Ship11109 + Ship11209
     + Ship12409 + Ship13009 + Ship14009 + Ship14109 + Ship14409
     + Ship14709 + Ship16709 + Ship17909 ;
  BOMConstr092: Ship30980  =  Ship20209 + Ship22309 + Ship22609
     + Ship22709 + Ship23109 + Ship25109 + Ship25509 + Ship25909
     + Ship26309 + Ship26909 + Ship27409 + Ship27909 ;
  BOMConstr101: 2 Ship31080  =  Ship10510 + Ship11110 + Ship12410
     + Ship13010 + Ship14010 + Ship14310 + Ship14410 + Ship15010
     + Ship17910 ;
  BOMConstr102: Ship31080  =  Ship20210 + Ship21610 + Ship22610
     + Ship23110 + Ship23210 + Ship26310 + Ship26910 + Ship27710
     + Ship27910 ;
  BOMConstr111: 2 Ship31180  =  Ship10111 + Ship10511 + Ship11111
     + Ship12411 + Ship13411 + Ship14311 + Ship14511 + Ship16111
     + Ship17911 ;
  BOMConstr112: Ship31180  =  Ship21611 + Ship23411 + Ship23811
     + Ship24511 + Ship25111 + Ship25911 + Ship26111 + Ship27711
     + Ship27811 + Ship27911 ;
  BOMConstr121: 2 Ship31280  =  Ship10112 + Ship11212 + Ship14012
     + Ship14112 + Ship14512 ;
  BOMConstr122: Ship31280  =  Ship20212 + Ship23112 + Ship23812
     + Ship24512 + Ship25112 + Ship25512 ;
  BOMConstr131: 2 Ship31380  =  Ship10113 + Ship12113 + Ship12413
     + Ship13013 + Ship14113 + Ship14713 + Ship16013 + Ship17913 ;
  BOMConstr132: Ship31380  =  Ship21613 + Ship23113 + Ship23213
     + Ship26313 + Ship26913 + Ship27913 ;
  BOMConstr141: 2 Ship31480  =  Ship10514 + Ship11114 + Ship11414
     + Ship14014 + Ship14114 + Ship14314 + Ship14414 + Ship14514
     + Ship15014 + Ship16014 + Ship16114 + Ship17914 ;
  BOMConstr142: Ship31480  =  Ship20214 + Ship22314 + Ship23814
     + Ship24514 + Ship25514 + Ship26114 + Ship26314 + Ship26914
     + Ship27414 + Ship27914 ;
  BOMConstr161: 2 Ship31680  =  Ship10116 + Ship14016 + Ship14316
     + Ship14516 + Ship14716 ;
  BOMConstr162: Ship31680  =  Ship21616 + Ship22716 + Ship23116
     + Ship23216 + Ship23816 + Ship24516 + Ship25116 + Ship26316
     + Ship27416 + Ship27716 ;
  BOMConstr171: 2 Ship31780  =  Ship11117 + Ship12417 + Ship14017
     + Ship14117 + Ship14717 + Ship15017 + Ship17917 ;
  BOMConstr172: Ship31780  =  Ship20217 + Ship22317 + Ship23217
     + Ship25117 + Ship26317 + Ship26917 + Ship27917 ;
  BOMConstr181: 2 Ship31880  =  Ship11218 + Ship11418 + Ship14018
     + Ship14218 + Ship14418 + Ship14518 + Ship14718 + Ship17918 ;
  BOMConstr182: Ship31880  =  Ship20218 + Ship22318 + Ship22618
     + Ship23118 + Ship23218 + Ship23818 + Ship24518 + Ship25918
     + Ship26318 + Ship26918 + Ship27918 ;
  BOMConstr191: 2 Ship31980  =  Ship10119 + Ship13019 + Ship14019
     + Ship14119 + Ship14319 + Ship14419 + Ship14719 + Ship16019
     + Ship16119 + Ship17919 ;
  BOMConstr192: Ship31980  =  Ship20219 + Ship23119 + Ship23219
     + Ship25119 + Ship25519 + Ship26119 + Ship26319 + Ship26919
     + Ship27719 + Ship27919 ;
  BOMConstr201: 2 Ship32080  =  Ship10120 + Ship10520 + Ship11120
     + Ship11220 + Ship12120 + Ship14020 + Ship14720 + Ship16020
     + Ship17920 ;
  BOMConstr202: Ship32080  =  Ship20220 + Ship22620 + Ship22720
     + Ship23120 + Ship23820 + Ship25120 + Ship26320 + Ship26920
     + Ship27420 + Ship27920 ;
  BOMConstr211: 2 Ship32180  =  Ship10121 + Ship11121 + Ship12121
     + Ship14021 + Ship14521 + Ship17921 ;
  BOMConstr212: Ship32180  =  Ship21621 + Ship24521 + Ship25121
     + Ship26921 + Ship27421 + Ship27821 + Ship27921 ;
  BOMConstr221: 2 Ship32280  =  Ship12422 + Ship14022 + Ship14322
     + Ship14422 + Ship16122 + Ship17922 ;
  BOMConstr222: Ship32280  =  Ship20222 + Ship21622 + Ship22722
     + Ship23222 + Ship25922 + Ship26122 + Ship26322 + Ship26922
     + Ship27922 ;
  BOMConstr231: 2 Ship32380  =  Ship10523 + Ship11123 + Ship11223
     + Ship12123 + Ship12423 + Ship14023 + Ship14423 + Ship14723
     + Ship17923 ;
  BOMConstr232: Ship32380  =  Ship20223 + Ship22323 + Ship22623
     + Ship22723 + Ship23823 + Ship25123 + Ship26323 + Ship26923
     + Ship27423 + Ship27923 ;
  BOMConstr241: 2 Ship32480  =  Ship11424 + Ship12424 + Ship14024
     + Ship16024 + Ship16124 + Ship17924 ;
  BOMConstr242: Ship32480  =  Ship20224 + Ship21624 + Ship22624
     + Ship22724 + Ship25124 + Ship26124 + Ship26924 + Ship27424
     + Ship27924 ;
  BOMConstr251: 2 Ship32580  =  Ship10525 + Ship11125 + Ship11225
     + Ship11425 + Ship12125 + Ship13025 + Ship14025 + Ship14725
     + Ship17925 ;
  BOMConstr252: Ship32580  =  Ship20225 + Ship21625 + Ship23125
     + Ship23225 + Ship25125 + Ship25525 + Ship26325 + Ship26925
     + Ship27925 ;
  BOMConstr281: 2 Ship32880  =  Ship11128 + Ship12128 + Ship13028
     + Ship14028 + Ship14428 + Ship16128 + Ship17928 ;
  BOMConstr282: Ship32880  =  Ship20228 + Ship21628 + Ship23128
     + Ship23828 + Ship25128 + Ship26128 + Ship26328 + Ship26928
     + Ship27928 ;
  BOMConstr291: 2 Ship32980  =  Ship12129 + Ship12429 + Ship13029
     + Ship14029 + Ship14529 + Ship16129 + Ship16729 + Ship17929 ;
  BOMConstr292: Ship32980  =  Ship20229 + Ship22329 + Ship23129
     + Ship23229 + Ship23829 + Ship24529 + Ship25129 + Ship25529
     + Ship26129 + Ship26329 + Ship26929 + Ship27429 + Ship27829
     + Ship27929 ;
  BOMConstr301: 2 Ship33080  =  Ship11130 + Ship13030 + Ship14030
     + Ship14330 + Ship14530 + Ship17930 ;
  BOMConstr302: Ship33080  =  Ship22330 + Ship24530 + Ship25130
     + Ship26930 + Ship27830 + Ship27930 ;
  BOMConstr311: 2 Ship33180  =  Ship11131 + Ship12131 + Ship12431
     + Ship13031 + Ship14031 + Ship16031 + Ship17931 ;
  BOMConstr312: Ship33180  =  Ship20231 + Ship22331 + Ship22731
     + Ship23131 + Ship23831 + Ship26331 + Ship26931 + Ship27931 ;
  BOMConstr331: 2 Ship33380  =  Ship11133 + Ship12133 + Ship12433
     + Ship14033 + Ship14433 + Ship14733 + Ship17933 ;
  BOMConstr332: Ship33380  =  Ship21633 + Ship22733 + Ship23133
     + Ship25133 + Ship26333 + Ship26933 + Ship27733 + Ship27933 ;
  BOMConstr351: 2 Ship33580  =  Ship10135 + Ship11235 + Ship11435
     + Ship11535 + Ship12135 + Ship12435 + Ship13035 + Ship14035
     + Ship15035 + Ship16135 + Ship17935 ;
  BOMConstr352: Ship33580  =  Ship20235 + Ship21635 + Ship22635
     + Ship22735 + Ship23135 + Ship23235 + Ship25135 + Ship26135
     + Ship26335 + Ship26935 + Ship27935 ;
  BOMConstr361: 2 Ship33680  =  Ship10136 + Ship11436 + Ship14036
     + Ship14136 + Ship15036 + Ship16136 + Ship17936 ;
  BOMConstr362: Ship33680  =  Ship23136 + Ship23236 + Ship23836
     + Ship25136 + Ship25536 + Ship26136 + Ship26336 + Ship26936
     + Ship27436 + Ship27736 + Ship27836 + Ship27936 ;
  BOMConstr371: 2 Ship33780  =  Ship10137 + Ship11537 + Ship13037
     + Ship14037 + Ship14137 + Ship14237 + Ship14337 + Ship14437
     + Ship14737 + Ship15037 + Ship16037 + Ship17937 ;
  BOMConstr372: Ship33780  =  Ship20237 + Ship21637 + Ship22337
     + Ship22637 + Ship23137 + Ship25137 + Ship25537 + Ship26337
     + Ship26937 + Ship27437 + Ship27837 + Ship27937 ;
  BOMConstr391: 2 Ship33980  =  Ship13439 + Ship14039 + Ship14139
     + Ship14439 + Ship14739 + Ship16039 + Ship17939 ;
  BOMConstr392: Ship33980  =  Ship20239 + Ship23139 + Ship23439
     + Ship25139 + Ship25539 + Ship26339 + Ship26939 + Ship27439
     + Ship27939 ;
  BOMConstr411: 2 Ship34180  =  Ship11141 + Ship14041 + Ship14141
     + Ship17941 ;
  BOMConstr412: Ship34180  =  Ship20241 + Ship21641 + Ship22341
     + Ship22741 + Ship23141 + Ship23841 + Ship25141 + Ship27941 ;
  BOMConstr421: 2 Ship34280  =  Ship10142 + Ship11142 + Ship11242
     + Ship11542 + Ship14242 + Ship14342 + Ship14742 + Ship16142
     + Ship17942 ;
  BOMConstr422: Ship34280  =  Ship20242 + Ship22342 + Ship22642
     + Ship22742 + Ship23842 + Ship25542 + Ship25942 + Ship26142
     + Ship26342 + Ship26942 + Ship27742 + Ship27942 ;
  BOMConstr441: 2 Ship34480  =  Ship11144 + Ship13044 + Ship14044
     + Ship14144 + Ship14344 + Ship14444 + Ship14544 + Ship16144
     + Ship17944 ;
  BOMConstr442: Ship34480  =  Ship20244 + Ship22744 + Ship23144
     + Ship23244 + Ship23844 + Ship24544 + Ship25144 + Ship26144
     + Ship26344 + Ship26944 + Ship27444 + Ship27744 + Ship27944 ;
  BOMConstr461: 2 Ship34680  =  Ship11146 + Ship13046 + Ship14046
     + Ship14446 + Ship14746 + Ship17946 ;
  BOMConstr462: Ship34680  =  Ship20246 + Ship22746 + Ship23146
     + Ship23246 + Ship25946 + Ship26346 + Ship26946 + Ship27946 ;
  BOMConstr471: 2 Ship34780  =  Ship10547 + Ship11147 + Ship11447
     + Ship12147 + Ship12447 + Ship13047 + Ship14447 + Ship14547
     + Ship14747 + Ship17947 ;
  BOMConstr472: Ship34780  =  Ship22647 + Ship23147 + Ship24547
     + Ship25147 + Ship27847 + Ship27947 ;
  BOMConstr481: 2 Ship34880  =  Ship10148 + Ship11148 + Ship12148
     + Ship13048 + Ship14048 + Ship14148 + Ship14548 + Ship16048 ;
  BOMConstr482: Ship34880  =  Ship20248 + Ship21648 + Ship22348
     + Ship23248 + Ship24548 + Ship25148 + Ship25548 + Ship25948
     + Ship26348 + Ship26948 + Ship27448 ;
  BOMConstr491: 2 Ship34980  =  Ship11249 + Ship11449 + Ship12149
     + Ship13049 + Ship14049 + Ship14149 + Ship14249 + Ship14449
     + Ship14549 + Ship14749 + Ship16149 + Ship17949 ;
  BOMConstr492: Ship34980  =  Ship22749 + Ship23149 + Ship23249
     + Ship23849 + Ship24549 + Ship25149 + Ship25549 + Ship26149
     + Ship26349 + Ship26949 + Ship27449 + Ship27949 ;
  BOMConstr501: 2 Ship35080  =  Ship12150 + Ship12450 + Ship13050
     + Ship14050 + Ship14150 + Ship14450 + Ship14750 + Ship15050
     + Ship17950 ;
  BOMConstr502: Ship35080  =  Ship22350 + Ship22750 + Ship23150
     + Ship23250 + Ship25150 + Ship25550 + Ship26950 + Ship27450
     + Ship27850 + Ship27950 ;
  BOMConstr521: 2 Ship35280  =  Ship10152 + Ship10552 + Ship11152
     + Ship13052 + Ship14052 + Ship14252 + Ship17952 ;
  BOMConstr522: Ship35280  =  Ship20252 + Ship22352 + Ship23152
     + Ship23252 + Ship25152 + Ship26352 + Ship26952 + Ship27452
     + Ship27852 + Ship27952 ;
  BOMConstr531: 2 Ship35380  =  Ship10553 + Ship11153 + Ship14053
     + Ship14153 + Ship14453 + Ship16053 + Ship16153 + Ship16753
     + Ship17953 ;
  BOMConstr532: Ship35380  =  Ship22753 + Ship23153 + Ship23253
     + Ship25153 + Ship26153 + Ship26953 + Ship27453 + Ship27753
     + Ship27953 ;
  BOMConstr541: 2 Ship35480  =  Ship11254 + Ship13054 + Ship14054
     + Ship14554 + Ship15054 + Ship16154 + Ship17954 ;
  BOMConstr542: Ship35480  =  Ship20254 + Ship22654 + Ship22754
     + Ship23154 + Ship24554 + Ship25154 + Ship25554 + Ship26154
     + Ship26354 + Ship26954 + Ship27454 + Ship27954 ;
  BOMConstr551: 2 Ship35580  =  Ship10555 + Ship11555 + Ship12455
     + Ship13055 + Ship14055 + Ship14155 + Ship14355 + Ship14455
     + Ship14755 + Ship16155 + Ship17955 ;
  BOMConstr552: Ship35580  =  Ship21655 + Ship23155 + Ship23855
     + Ship25155 + Ship25555 + Ship25955 + Ship26155 + Ship26955
     + Ship27455 + Ship27955 ;
  BOMConstr561: 2 Ship35680  =  Ship10156 + Ship11156 + Ship11256
     + Ship12156 + Ship12456 + Ship13056 + Ship14056 + Ship14356
     + Ship14556 + Ship16156 + Ship16756 ;
  BOMConstr562: Ship35680  =  Ship22656 + Ship24556 + Ship25156
     + Ship25556 + Ship25956 + Ship26156 + Ship26356 + Ship26956
     + Ship27456 ;
  BOMConstr571: 2 Ship35780  =  Ship11157 + Ship11257 + Ship14057
     + Ship14157 + Ship14357 + Ship14457 + Ship14557 + Ship14757
     + Ship15057 + Ship16157 ;
  BOMConstr572: Ship35780  =  Ship20257 + Ship21657 + Ship23257
     + Ship23857 + Ship24557 + Ship25157 + Ship25557 + Ship25957
     + Ship26157 + Ship26357 + Ship26957 + Ship27457 + Ship27857 ;
  BOMConstr581: 2 Ship35880  =  Ship13058 + Ship14058 + Ship14158
     + Ship14358 + Ship14458 + Ship14558 + Ship14758 + Ship16058
     + Ship17958 ;
  BOMConstr582: Ship35880  =  Ship20258 + Ship22758 + Ship24558
     + Ship25158 + Ship26358 + Ship26958 + Ship27458 + Ship27958 ;
  BOMConstr591: 2 Ship35980  =  Ship10559 + Ship14059 + Ship14459
     + Ship14559 + Ship16159 + Ship17959 ;
  BOMConstr592: Ship35980  =  Ship20259 + Ship22759 + Ship23159
     + Ship23859 + Ship24559 + Ship25159 + Ship25959 + Ship26159
     + Ship26359 + Ship27959 ;
  BOMConstr601: 2 Ship36080  =  Ship11160 + Ship11260 + Ship13060
     + Ship13460 + Ship14060 + Ship14160 + Ship14760 + Ship16060 ;
  BOMConstr602: Ship36080  =  Ship22760 + Ship23460 + Ship25160
     + Ship25560 + Ship26360 + Ship26960 + Ship27460 ;
  BOMConstr621: 2 Ship36280  =  Ship11162 + Ship11262 + Ship13062
     + Ship14062 + Ship14162 + Ship14562 + Ship14762 + Ship15062
     + Ship16062 + Ship17962 ;
  BOMConstr622: Ship36280  =  Ship21662 + Ship22362 + Ship22762
     + Ship23162 + Ship23862 + Ship24562 + Ship25162 + Ship26362
     + Ship26962 + Ship27462 + Ship27962 ;
  BOMConstr631: 2 Ship36380  =  Ship11163 + Ship11263 + Ship11563
     + Ship14063 + Ship14163 + Ship14363 + Ship14763 + Ship17963 ;
  BOMConstr632: Ship36380  =  Ship20263 + Ship22763 + Ship23863
     + Ship25163 + Ship25563 + Ship26363 + Ship27463 + Ship27963 ;
  BOMConstr641: 2 Ship36480  =  Ship11164 + Ship13064 + Ship14264
     + Ship14364 + Ship14764 + Ship15064 + Ship16164 + Ship17964 ;
  BOMConstr642: Ship36480  =  Ship20264 + Ship21664 + Ship22364
     + Ship22664 + Ship23164 + Ship23864 + Ship25164 + Ship26164
     + Ship26364 + Ship26964 + Ship27464 + Ship27864 + Ship27964 ;
  BOMConstr651: 2 Ship36580  =  Ship10165 + Ship12465 + Ship14465
     + Ship14565 + Ship14765 + Ship17965 ;
  BOMConstr652: Ship36580  =  Ship20265 + Ship21665 + Ship22765
     + Ship23165 + Ship23865 + Ship24565 + Ship26365 + Ship26965
     + Ship27465 + Ship27765 + Ship27965 ;
  BOMConstr661: 2 Ship36680  =  Ship11466 + Ship12466 + Ship13066
     + Ship14066 + Ship14166 + Ship14266 + Ship14366 + Ship15066
     + Ship16166 ;
  BOMConstr662: Ship36680  =  Ship20266 + Ship21666 + Ship23166
     + Ship23266 + Ship23866 + Ship25166 + Ship26166 + Ship26966
     + Ship27466 ;
  BOMConstr681: 2 Ship36880  =  Ship10568 + Ship11168 + Ship11468
     + Ship12468 + Ship13068 + Ship14068 + Ship14268 + Ship14768
     + Ship15068 + Ship16168 + Ship17968 ;
  BOMConstr682: Ship36880  =  Ship20268 + Ship21668 + Ship22368
     + Ship23168 + Ship23868 + Ship26168 + Ship26968 + Ship27468
     + Ship27968 ;
  BOMConstr691: 2 Ship36980  =  Ship11169 + Ship11269 + Ship14069
     + Ship14169 + Ship14369 + Ship14569 + Ship14769 + Ship17969 ;
  BOMConstr692: Ship36980  =  Ship20269 + Ship21669 + Ship22769
     + Ship23269 + Ship23869 + Ship24569 + Ship26369 + Ship26969
     + Ship27969 ;
  BOMConstr701: 2 Ship37080  =  Ship10570 + Ship13070 + Ship14070
     + Ship14570 + Ship17970 ;
  BOMConstr702: Ship37080  =  Ship20270 + Ship21670 + Ship22770
     + Ship23170 + Ship23270 + Ship24570 + Ship25170 + Ship26970
     + Ship27970 ;
  BOMConstr711: 2 Ship37180  =  Ship11171 + Ship11471 + Ship11571
     + Ship12171 + Ship12471 + Ship14071 + Ship14171 + Ship14371
     + Ship14771 + Ship16171 ;
  BOMConstr712: Ship37180  =  Ship20271 + Ship23171 + Ship23271
     + Ship23871 + Ship25171 + Ship26171 + Ship26371 + Ship26971
     + Ship27471 ;
  BOMConstr721: 2 Ship37280  =  Ship11172 + Ship12472 + Ship13072
     + Ship14372 + Ship14472 + Ship14572 + Ship14772 + Ship15072
     + Ship16072 ;
  BOMConstr722: Ship37280  =  Ship21672 + Ship22372 + Ship23172
     + Ship23272 + Ship23872 + Ship24572 + Ship25972 + Ship26372
     + Ship26972 + Ship27472 + Ship27772 ;
  BOMConstr731: 2 Ship37380  =  Ship10173 + Ship10573 + Ship11173
     + Ship12173 + Ship12473 + Ship14073 + Ship14173 + Ship14373
     + Ship14473 + Ship15073 + Ship16773 + Ship17973 ;
  BOMConstr732: Ship37380  =  Ship22373 + Ship22673 + Ship22773
     + Ship23173 + Ship23873 + Ship25173 + Ship25573 + Ship26373
     + Ship26973 + Ship27473 + Ship27873 + Ship27973 ;
  BOMConstr751: 2 Ship37580  =  Ship10175 + Ship11275 + Ship12175
     + Ship12475 + Ship13075 + Ship14075 + Ship14175 + Ship14475
     + Ship15075 + Ship17975 ;
  BOMConstr752: Ship37580  =  Ship20275 + Ship21675 + Ship22375
     + Ship23175 + Ship23275 + Ship25175 + Ship25575 + Ship26375
     + Ship26975 + Ship27975 ;
  BOMConstr761: 2 Ship37680  =  Ship10176 + Ship11176 + Ship11576
     + Ship13076 + Ship14076 + Ship14176 + Ship14576 + Ship17976 ;
  BOMConstr762: Ship37680  =  Ship20276 + Ship23176 + Ship23876
     + Ship24576 + Ship25576 + Ship25976 + Ship26976 + Ship27476
     + Ship27976 ;
  BOMConstr771: 2 Ship37780  =  Ship12477 + Ship13077 + Ship14077
     + Ship14577 + Ship14777 + Ship16177 ;
  BOMConstr772: Ship37780  =  Ship22777 + Ship23177 + Ship24577
     + Ship25177 + Ship25577 + Ship25977 + Ship26177 + Ship26377
     + Ship26977 + Ship27777 ;
  DemConstr: Ship30180 + Ship30280 + Ship30380 + Ship30480 + Ship30580
     + Ship30680 + Ship30780 + Ship30880 + Ship30980 + Ship31080
     + Ship31180 + Ship31280 + Ship31380 + Ship31480 + Ship31680
     + Ship31780 + Ship31880 + Ship31980 + Ship32080 + Ship32180
     + Ship32280 + Ship32380 + Ship32480 + Ship32580 + Ship32880
     + Ship32980 + Ship33080 + Ship33180 + Ship33380 + Ship33580
     + Ship33680 + Ship33780 + Ship33980 + Ship34180 + Ship34280
     + Ship34480 + Ship34680 + Ship34780 + Ship34880 + Ship34980
     + Ship35080 + Ship35280 + Ship35380 + Ship35480 + Ship35580
     + Ship35680 + Ship35780 + Ship35880 + Ship35980 + Ship36080
     + Ship36280 + Ship36380 + Ship36480 + Ship36580 + Ship36680
     + Ship36880 + Ship36980 + Ship37080 + Ship37180 + Ship37280
     + Ship37380 + Ship37580 + Ship37680 + Ship37780  <=  51808 ;
  PlantConstr01: Ship30180  <=  5000 OpenProduction01 ;
  PlantConstr02: Ship30280  <=  5000 OpenProduction02 ;
  PlantConstr03: Ship30380  <=  5000 OpenProduction03 ;
  PlantConstr04: Ship30480  <=  5000 OpenProduction04 ;
  PlantConstr05: Ship30580  <=  5000 OpenProduction05 ;
  PlantConstr06: Ship30680  <=  5000 OpenProduction06 ;
  PlantConstr07: Ship30780  <=  5000 OpenProduction07 ;
  PlantConstr08: Ship30880  <=  5000 OpenProduction08 ;
  PlantConstr09: Ship30980  <=  5000 OpenProduction09 ;
  PlantConstr10: Ship31080  <=  5000 OpenProduction10 ;
  PlantConstr11: Ship31180  <=  5000 OpenProduction11 ;
  PlantConstr12: Ship31280  <=  5000 OpenProduction12 ;
  PlantConstr13: Ship31380  <=  5000 OpenProduction13 ;
  PlantConstr14: Ship31480  <=  5000 OpenProduction14 ;
  PlantConstr16: Ship31680  <=  5000 OpenProduction16 ;
  PlantConstr17: Ship31780  <=  5000 OpenProduction17 ;
  PlantConstr18: Ship31880  <=  5000 OpenProduction18 ;
  PlantConstr19: Ship31980  <=  5000 OpenProduction19 ;
  PlantConstr20: Ship32080  <=  5000 OpenProduction20 ;
  PlantConstr21: Ship32180  <=  5000 OpenProduction21 ;
  PlantConstr22: Ship32280  <=  5000 OpenProduction22 ;
  PlantConstr23: Ship32380  <=  5000 OpenProduction23 ;
  PlantConstr24: Ship32480  <=  5000 OpenProduction24 ;
  PlantConstr25: Ship32580  <=  5000 OpenProduction25 ;
  PlantConstr28: Ship32880  <=  5000 OpenProduction28 ;
  PlantConstr29: Ship32980  <=  5000 OpenProduction29 ;
  PlantConstr30: Ship33080  <=  5000 OpenProduction30 ;
  PlantConstr31: Ship33180  <=  5000 OpenProduction31 ;
  PlantConstr33: Ship33380  <=  5000 OpenProduction33 ;
  PlantConstr35: Ship33580  <=  5000 OpenProduction35 ;
  PlantConstr36: Ship33680  <=  5000 OpenProduction36 ;
  PlantConstr37: Ship33780  <=  5000 OpenProduction37 ;
  PlantConstr39: Ship33980  <=  5000 OpenProduction39 ;
  PlantConstr41: Ship34180  <=  5000 OpenProduction41 ;
  PlantConstr42: Ship34280  <=  5000 OpenProduction42 ;
  PlantConstr44: Ship34480  <=  5000 OpenProduction44 ;
  PlantConstr46: Ship34680  <=  5000 OpenProduction46 ;
  PlantConstr47: Ship34780  <=  5000 OpenProduction47 ;
  PlantConstr48: Ship34880  <=  5000 OpenProduction48 ;
  PlantConstr49: Ship34980  <=  5000 OpenProduction49 ;
  PlantConstr50: Ship35080  <=  5000 OpenProduction50 ;
  PlantConstr52: Ship35280  <=  5000 OpenProduction52 ;
  PlantConstr53: Ship35380  <=  5000 OpenProduction53 ;
  PlantConstr54: Ship35480  <=  5000 OpenProduction54 ;
  PlantConstr55: Ship35580  <=  5000 OpenProduction55 ;
  PlantConstr56: Ship35680  <=  5000 OpenProduction56 ;
  PlantConstr57: Ship35780  <=  5000 OpenProduction57 ;
  PlantConstr58: Ship35880  <=  5000 OpenProduction58 ;
  PlantConstr59: Ship35980  <=  5000 OpenProduction59 ;
  PlantConstr60: Ship36080  <=  5000 OpenProduction60 ;
  PlantConstr62: Ship36280  <=  5000 OpenProduction62 ;
  PlantConstr63: Ship36380  <=  5000 OpenProduction63 ;
  PlantConstr64: Ship36480  <=  5000 OpenProduction64 ;
  PlantConstr65: Ship36580  <=  5000 OpenProduction65 ;
  PlantConstr66: Ship36680  <=  5000 OpenProduction66 ;
  PlantConstr68: Ship36880  <=  5000 OpenProduction68 ;
  PlantConstr69: Ship36980  <=  5000 OpenProduction69 ;
  PlantConstr70: Ship37080  <=  5000 OpenProduction70 ;
  PlantConstr71: Ship37180  <=  5000 OpenProduction71 ;
  PlantConstr72: Ship37280  <=  5000 OpenProduction72 ;
  PlantConstr73: Ship37380  <=  5000 OpenProduction73 ;
  PlantConstr75: Ship37580  <=  5000 OpenProduction75 ;
  PlantConstr76: Ship37680  <=  5000 OpenProduction76 ;
  PlantConstr77: Ship37780  <=  5000 OpenProduction77 ;

BINARY
    OpenProduction01
    OpenProduction02
    OpenProduction03
    OpenProduction04
    OpenProduction05
    OpenProduction06
    OpenProduction07
    OpenProduction08
    OpenProduction09
    OpenProduction10
    OpenProduction11
    OpenProduction12
    OpenProduction13
    OpenProduction14
    OpenProduction16
    OpenProduction17
    OpenProduction18
    OpenProduction19
    OpenProduction20
    OpenProduction21
    OpenProduction22
    OpenProduction23
    OpenProduction24
    OpenProduction25
    OpenProduction28
    OpenProduction29
    OpenProduction30
    OpenProduction31
    OpenProduction33
    OpenProduction35
    OpenProduction36
    OpenProduction37
    OpenProduction39
    OpenProduction41
    OpenProduction42
    OpenProduction44
    OpenProduction46
    OpenProduction47
    OpenProduction48
    OpenProduction49
    OpenProduction50
    OpenProduction52
    OpenProduction53
    OpenProduction54
    OpenProduction55
    OpenProduction56
    OpenProduction57
    OpenProduction58
    OpenProduction59
    OpenProduction60
    OpenProduction62
    OpenProduction63
    OpenProduction64
    OpenProduction65
    OpenProduction66
    OpenProduction68
    OpenProduction69
    OpenProduction70
    OpenProduction71
    OpenProduction72
    OpenProduction73
    OpenProduction75
    OpenProduction76
    OpenProduction77

END

