//For cc_regdb, get mcc to wlan_country
struct mcc_to_country_code {
    int mcc;
    //uint8_t alpha[3];
    char alpha[3];
};

//int main()
//{
  /*
   * The table below is built from two resources:
   *
   * 1) ITU "Mobile Network Code (MNC) for the international
   *   identification plan for mobile terminals and mobile users"
   *   which is available as an annex to the ITU operational bulletin
   *   available here: http://www.itu.int/itu-t/bulletin/annex.html
   *
   * 2) The ISO 3166 country codes list, available here:
   *    http://www.iso.org/iso/en/prods-services/iso3166ma/02iso-3166-code-lists/index.html
   *
   * This table has not been verified.
  */
  const struct mcc_to_country_code all_country_list[] = {
      {202,"GR"},	//Greece
      {204,"NL"},	//Netherlands (Kingdom of the)
      {206,"BE"},	//Belgium
      {208,"FR"},	//France
      {212,"MC"},	//Monaco (Principality of)
      {213,"AD"},	//Andorra (Principality of)
      {214,"ES"},	//Spain
      {216,"HU"},	//Hungary (Republic of)
      {218,"BA"},	//Bosnia and Herzegovina
      {219,"HR"},	//Croatia (Republic of)
      {220,"RS"},	//Serbia and Montenegro
      {222,"IT"},	//Italy
      {225,"VA"},	//Vatican City State
      {226,"RO"},	//Romania
      {228,"CH"},	//Switzerland (Confederation of)
      {230,"CZ"},	//Czechia
      {231,"SK"},	//Slovak Republic
      {232,"AT"},	//Austria
      {234,"GB"},	//United Kingdom of Great Britain and Northern Ireland
      {235,"GB"},	//United Kingdom of Great Britain and Northern Ireland
      {238,"DK"},	//Denmark
      {240,"SE"},	//Sweden
      {242,"NO"},	//Norway
      {244,"FI"},	//Finland
      {246,"LT"},	//Lithuania (Republic of)
      {247,"LV"},	//Latvia (Republic of)
      {248,"EE"},	//Estonia (Republic of)
      {250,"RU"},	//Russian Federation
      {255,"UA"},	//Ukraine
      {257,"BY"},	//Belarus (Republic of)
      {259,"MD"},	//Moldova (Republic of)
      {260,"PL"},	//Poland (Republic of)
      {262,"DE"},	//Germany (Federal Republic of)
      {266,"GI"},	//Gibraltar
      {268,"PT"},	//Portugal
      {270,"LU"},	//Luxembourg
      {272,"IE"},	//Ireland
      {274,"IS"},	//Iceland
      {276,"AL"},	//Albania (Republic of)
      {278,"MT"},	//Malta
      {280,"CY"},	//Cyprus (Republic of)
      {282,"GE"},	//Georgia
      {283,"AM"},	//Armenia (Republic of)
      {284,"BG"},	//Bulgaria (Republic of)
      {286,"TR"},	//Turkey
      {288,"FO"},	//Faroe Islands
      {289,"GE"},    //Abkhazia (Georgia)
      {290,"GL"},	//Greenland (Denmark)
      {292,"SM"},	//San Marino (Republic of)
      {293,"SI"},	//Slovenia (Republic of)
      {294,"MK"},   //The Former Yugoslav Republic of Macedonia
      {295,"LI"},	//Liechtenstein (Principality of)
      {297,"ME"},    //Montenegro (Republic of)
      {302,"CA"},	//Canada
      {308,"PM"},	//Saint Pierre and Miquelon (Collectivit territoriale de la Rpublique franaise)
      {310,"US"},	//United States of America
      {311,"US"},	//United States of America
      {312,"US"},	//United States of America
      {313,"US"},	//United States of America
      {314,"US"},	//United States of America
      {315,"US"},	//United States of America
      {316,"US"},	//United States of America
      {330,"PR"},	//Puerto Rico
      {332,"VI"},	//United States Virgin Islands
      {334,"MX"},	//Mexico
      {338,"JM"},	//Jamaica
      {340,"GP"},	//Guadeloupe (French Department of)
      {342,"BB"},	//Barbados
      {344,"AG"},	//Antigua and Barbuda
      {346,"KY"},	//Cayman Islands
      {348,"VG"},	//British Virgin Islands
      {350,"BM"},	//Bermuda
      {352,"GD"},	//Grenada
      {354,"MS"},	//Montserrat
      {356,"KN"},	//Saint Kitts and Nevis
      {358,"LC"},	//Saint Lucia
      {360,"VC"},	//Saint Vincent and the Grenadines
      {362,"AN"},	//Netherlands Antilles
      {363,"AW"},	//Aruba
      {364,"BS"},	//Bahamas (Commonwealth of the)
      {365,"AI"},	//Anguilla
      {366,"DM"},	//Dominica (Commonwealth of)
      {368,"CU"},	//Cuba
      {370,"DO"},	//Dominican Republic
      {372,"HT"},	//Haiti (Republic of)
      {374,"TT"},	//Trinidad and Tobago
      {376,"TC"},	//Turks and Caicos Islands
      {400,"AZ"},	//Azerbaijani Republic
      {401,"KZ"},	//Kazakhstan (Republic of)
      {402,"BT"},	//Bhutan (Kingdom of)
      {404,"IN"},	//India (Republic of)
      {405,"IN"},	//India (Republic of)
      {406,"IN"},	//India (Republic of)
      {410,"PK"},	//Pakistan (Islamic Republic of)
      {412,"AF"},	//Afghanistan
      {413,"LK"},	//Sri Lanka (Democratic Socialist Republic of)
      {414,"MM"},	//Myanmar (Union of)
      {415,"LB"},	//Lebanon
      {416,"JO"},	//Jordan (Hashemite Kingdom of)
      {417,"SY"},	//Syrian Arab Republic
      {418,"IQ"},	//Iraq (Republic of)
      {419,"KW"},	//Kuwait (State of)
      {420,"SA"},	//Saudi Arabia (Kingdom of)
      {421,"YE"},	//Yemen (Republic of)
      {422,"OM"},	//Oman (Sultanate of)
      {423,"PS"},	//Palestine
      {424,"AE"},	//United Arab Emirates
      {425,"IL"},	//Israel (State of)
      {426,"BH"},	//Bahrain (Kingdom of)
      {427,"QA"},	//Qatar (State of)
      {428,"MN"},	//Mongolia
      {429,"NP"},	//Nepal
      {430,"AE"},	//United Arab Emirates
      {431,"AE"},	//United Arab Emirates
      {432,"IR"},	//Iran (Islamic Republic of)
      {434,"UZ"},	//Uzbekistan (Republic of)
      {436,"TJ"},	//Tajikistan (Republic of)
      {437,"KG"},	//Kyrgyz Republic
      {438,"TM"},	//Turkmenistan
      {440,"JP"},	//Japan
      {441,"JP"},	//Japan
      {450,"KR"},	//Korea (Republic of)
      {452,"VN"},	//Viet Nam (Socialist Republic of)
      {454,"HK"},	//"Hong Kong, China"
      {455,"MO"},	//"Macao, China"
      {456,"KH"},	//Cambodia (Kingdom of)
      {457,"LA"},	//Lao People's Democratic Republic
      {460,"CN"},	//China (People's Republic of)
      {461,"CN"},	//China (People's Republic of)
      {466,"TW"},	//Taiwan
      {467,"KP"},	//Democratic People's Republic of Korea
      {470,"BD"},	//Bangladesh (People's Republic of)
      {472,"MV"},	//Maldives (Republic of)
      {502,"MY"},	//Malaysia
      {505,"AU"},	//Australia
      {510,"ID"},	//Indonesia (Republic of)
      {514,"TL"},	//Democratic Republic of Timor-Leste
      {515,"PH"},	//Philippines (Republic of the)
      {520,"TH"},	//Thailand
      {525,"SG"},	//Singapore (Republic of)
      {528,"BN"},	//Brunei Darussalam
      {530,"NZ"},	//New Zealand
      {534,"MP"},	//Northern Mariana Islands (Commonwealth of the)
      {535,"GU"},	//Guam
      {536,"NR"},	//Nauru (Republic of)
      {537,"PG"},	//Papua New Guinea
      {539,"TO"},	//Tonga (Kingdom of)
      {540,"SB"},	//Solomon Islands
      {541,"VU"},	//Vanuatu (Republic of)
      {542,"FJ"},	//Fiji (Republic of)
      {543,"WF"},	//Wallis and Futuna (Territoire franais d'outre-mer)
      {544,"AS"},	//American Samoa
      {545,"KI"},	//Kiribati (Republic of)
      {546,"NC"},	//New Caledonia (Territoire franais d'outre-mer)
      {547,"PF"},	//French Polynesia (Territoire franais d'outre-mer)
      {548,"CK"},	//Cook Islands
      {549,"WS"},	//Samoa (Independent State of)
      {550,"FM"},	//Micronesia (Federated States of)
      {551,"MH"},	//Marshall Islands (Republic of the)
      {552,"PW"},	//Palau (Republic of)
      {553,"TV"},	//Tuvalu
      {555,"NU"},	//Niue
      {602,"EG"},	//Egypt (Arab Republic of)
      {603,"DZ"},	//Algeria (People's Democratic Republic of)
      {604,"MA"},	//Morocco (Kingdom of)
      {605,"TN"},	//Tunisia
      {606,"LY"},	//Libya (Socialist People's Libyan Arab Jamahiriya)
      {607,"GM"},	//Gambia (Republic of the)
      {608,"SN"},	//Senegal (Republic of)
      {609,"MR"},	//Mauritania (Islamic Republic of)
      {610,"ML"},	//Mali (Republic of)
      {611,"GN"},	//Guinea (Republic of)
      {612,"CI"},	//Côte d'Ivoire (Republic of)
      {613,"BF"},	//Burkina Faso
      {614,"NE"},	//Niger (Republic of the)
      {615,"TG"},	//Togolese Republic
      {616,"BJ"},	//Benin (Republic of)
      {617,"MU"},	//Mauritius (Republic of)
      {618,"LR"},	//Liberia (Republic of)
      {619,"SL"},	//Sierra Leone
      {620,"GH"},	//Ghana
      {621,"NG"},	//Nigeria (Federal Republic of)
      {622,"TD"},	//Chad (Republic of)
      {623,"CF"},	//Central African Republic
      {624,"CM"},	//Cameroon (Republic of)
      {625,"CV"},	//Cape Verde (Republic of)
      {626,"ST"},	//Sao Tome and Principe (Democratic Republic of)
      {627,"GQ"},	//Equatorial Guinea (Republic of)
      {628,"GA"},	//Gabonese Republic
      {629,"CG"},	//Congo (Republic of the)
      {630,"CD"},	//Democratic Republic of the Congo
      {631,"AO"},	//Angola (Republic of)
      {632,"GW"},	//Guinea-Bissau (Republic of)
      {633,"SC"},	//Seychelles (Republic of)
      {634,"SD"},	//Sudan (Republic of the)
      {635,"RW"},	//Rwanda (Republic of)
      {636,"ET"},	//Ethiopia (Federal Democratic Republic of)
      {637,"SO"},	//Somali Democratic Republic
      {638,"DJ"},	//Djibouti (Republic of)
      {639,"KE"},	//Kenya (Republic of)
      {640,"TZ"},	//Tanzania (United Republic of)
      {641,"UG"},	//Uganda (Republic of)
      {642,"BI"},	//Burundi (Republic of)
      {643,"MZ"},	//Mozambique (Republic of)
      {645,"ZM"},	//Zambia (Republic of)
      {646,"MG"},	//Madagascar (Republic of)
      {647,"RE"},	//Reunion (French Department of)
      {648,"ZW"},	//Zimbabwe (Republic of)
      {649,"NA"},	//Namibia (Republic of)
      {650,"MW"},	//Malawi
      {651,"LS"},	//Lesotho (Kingdom of)
      {652,"BW"},	//Botswana (Republic of)
      {653,"SZ"},	//Swaziland (Kingdom of)
      {654,"KM"},	//Comoros (Union of the)
      {655,"ZA"},	//South Africa (Republic of)
      {657,"ER"},	//Eritrea
      {658,"SH"},	//Saint Helena, Ascension and Tristan da Cunha
      {659,"SS"},	//South Sudan (Republic of)
      {702,"BZ"},	//Belize
      {704,"GT"},	//Guatemala (Republic of)
      {706,"SV"},	//El Salvador (Republic of)
      {708,"HN"},	//Honduras (Republic of)
      {710,"NI"},	//Nicaragua
      {712,"CR"},	//Costa Rica
      {714,"PA"},	//Panama (Republic of)
      {716,"PE"},	//Peru
      {722,"AR"},	//Argentine Republic
      {724,"BR"},	//Brazil (Federative Republic of)
      {730,"CL"},	//Chile
      {732,"CO"},	//Colombia (Republic of)
      {734,"VE"},	//Venezuela (Bolivarian Republic of)
      {736,"BO"},	//Bolivia (Republic of)
      {738,"GY"},	//Guyana
      {740,"EC"},	//Ecuador
      {742,"GF"},	//French Guiana (French Department of)
      {744,"PY"},	//Paraguay (Republic of)
      {746,"SR"},	//Suriname (Republic of)
      {748,"UY"},	//Uruguay (Eastern Republic of)
      {750,"FK"},	//Falkland Islands (Malvinas)
      //{901,""},	//"International Mobile, shared code"
  };

