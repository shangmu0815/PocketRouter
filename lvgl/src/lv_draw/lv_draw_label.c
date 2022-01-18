/**
 * @file lv_draw_label.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "lv_draw_label.h"
#include "../lv_misc/lv_math.h"
#include "lv_pocket_router/src/util/debug_log.h"
#include "lv_pocket_router/src/util/util.h"

/*********************
 *      DEFINES
 *********************/
#define LABEL_RECOLOR_PAR_LENGTH 6
#define LV_LABEL_HINT_UPDATE_TH 1024 /*Update the "hint" if the label's y coordinates have changed more then this*/

/*
#ifdef FEATURE_ROUTER
#define AR_REVERSE_SUPPORT 0
#define AR_TABLE_TEST_ONLY 0
#else
#define USE_FRIBIDI 1
#define AR_REVERSE_SUPPORT 0
#define AR_TABLE_TEST_ONLY 0
#endif
*/
// [ZX53] Merge lvgl v7.8.1 partial code, enable LV_USE_ARABIC_PERSIAN_CHARS to use Arabic language parsing
#ifdef LV_USE_ARABIC_PERSIAN_CHARS
#define AR_REVERSE_SUPPORT 1
#define AR_TABLE_TEST_ONLY 0
#else
#define AR_REVERSE_SUPPORT 0
#define AR_TABLE_TEST_ONLY 0
#endif
/**********************
 *      TYPEDEFS
 **********************/
enum {
    CMD_STATE_WAIT,
    CMD_STATE_PAR,
    CMD_STATE_IN,
};
typedef uint8_t cmd_state_t;

#if (USE_FRIBIDI)
/* Each char can be only one of the six following. */
#define FRIBIDI_MASK_LETTER	0x00000100L	/* Is letter: L, R, AL */
#define FRIBIDI_MASK_NUMBER	0x00000200L	/* Is number: EN, AN */
#define FRIBIDI_MASK_NUMSEPTER	0x00000400L	/* Is separator or terminator: ES, ET, CS */
#define FRIBIDI_MASK_SPACE	0x00000800L	/* Is space: BN, BS, SS, WS */
#define FRIBIDI_MASK_EXPLICIT	0x00001000L	/* Is explicit mark: LRE, RLE, LRO, RLO, PDF */
#define FRIBIDI_MASK_ISOLATE	0x00008000L     /* Is isolate mark: LRI, RLI, FSI, PDI */

/* RTL mask better be the least significant bit. */
#define FRIBIDI_MASK_RTL	0x00000001L	/* Is right to left */
#define FRIBIDI_MASK_ARABIC	0x00000002L	/* Is arabic */

#define FRIBIDI_MASK_STRONG	0x00000010L	/* Is strong */
#define FRIBIDI_MASK_WEAK	0x00000020L	/* Is weak */
#define FRIBIDI_MASK_NEUTRAL	0x00000040L	/* Is neutral */
#define FRIBIDI_MASK_SENTINEL	0x00000080L	/* Is sentinel */

#define FRIBIDI_TYPE_RTL_VAL	( FRIBIDI_MASK_STRONG | FRIBIDI_MASK_LETTER \
				| FRIBIDI_MASK_RTL)

# define FRIBIDI_TYPE_RTL	FRIBIDI_TYPE_RTL_VAL
#endif /* USE_FRIBIDI */

/**********************
 *  STATIC PROTOTYPES
 **********************/
static uint8_t hex_char_to_num(char hex);
//[ZX53] check if txt include AR text start
static bool skip_non_rtl_reappend(const char * txt, uint32_t line_start);
static uint32_t count_non_rtl_begin(const char * txt, uint32_t line_start);
static uint32_t count_non_rtl_ending(const char * txt, uint32_t line_start, uint32_t line_end);
//[ZX53] check if txt include AR text end

/**********************
 *  STATIC VARIABLES
 **********************/
#if (AR_TABLE_TEST_ONLY)
// 0621 to 064a deformation code, exceed this part are their own codeword
static const uint16_t Arabic_PositionTable[][4]= {
// first, last, middle, alone
{ 0xfe80, 0xfe80, 0xfe80, 0xfe80 }, // 0x621
{ 0xfe82, 0xfe81, 0xfe82, 0xfe81 },
{ 0xfe84, 0xfe83, 0xfe84, 0xfe83 },
{ 0xfe86, 0xfe85, 0xfe86, 0xfe85 },
{ 0xfe88, 0xfe87, 0xfe88, 0xfe87 },
{ 0xfe8a, 0xfe8b, 0xfe8c, 0xfe89 },
{ 0xfe8e, 0xfe8d, 0xfe8e, 0xfe8d },
{ 0xfe90, 0xfe91, 0xfe92, 0xfe8f }, // 0x628
{ 0xfe94, 0xfe93, 0xfe93, 0xfe93 },
{ 0xfe96, 0xfe97, 0xfe98, 0xfe95 }, // 0x62A
{ 0xfe9a, 0xfe9b, 0xfe9c, 0xfe99 },
{ 0xfe9e, 0xfe9f, 0xfea0, 0xfe9d },
{ 0xfea2, 0xfea3, 0xfea4, 0xfea1 },
{ 0xfea6, 0xfea7, 0xfea8, 0xfea5 },
{ 0xfeaa, 0xfea9, 0xfeaa, 0xfea9 },
{ 0xfeac, 0xfeab, 0xfeac, 0xfeab }, // 0x630
{ 0xfeae, 0xfead, 0xfeae, 0xfead },
{ 0xfeb0, 0xfeaf, 0xfeb0, 0xfeaf },
{ 0xfeb2, 0xfeb3, 0xfeb4, 0xfeb1 },
{ 0xfeb6, 0xfeb7, 0xfeb8, 0xfeb5 },
{ 0xfeba, 0xfebb, 0xfebc, 0xfeb9 },
{ 0xfebe, 0xfebf, 0xfec0, 0xfebd },
{ 0xfec2, 0xfec3, 0xfec4, 0xfec1 },
{ 0xfec6, 0xfec7, 0xfec8, 0xfec5 }, // 0x638
{ 0xfeca, 0xfecb, 0xfecc, 0xfec9 },
{ 0xfece, 0xfecf, 0xfed0, 0xfecd }, //0x63A
{ 0x063b, 0x063b, 0x063b, 0x063b },
{ 0x063c, 0x063c, 0x063c, 0x063c },
{ 0x063d, 0x063d, 0x063d, 0x063d },
{ 0x063e, 0x063e, 0x063e, 0x063e },
{ 0x063f, 0x063f, 0x063f, 0x063f },
{ 0x0640, 0x0640, 0x0640, 0x0640 }, // 0x640
{ 0xfed2, 0xfed3, 0xfed4, 0xfed1 },
{ 0xfed6, 0xfed7, 0xfed8, 0xfed5 },
{ 0xfeda, 0xfedb, 0xfedc, 0xfed9 },
{ 0xfede, 0xfedf, 0xfee0, 0xfedd },
{ 0xfee2, 0xfee3, 0xfee4, 0xfee1 },
{ 0xfee6, 0xfee7, 0xfee8, 0xfee5 },
{ 0xfeea, 0xfeeb, 0xfeec, 0xfee9 },
{ 0xfeee, 0xfeed, 0xfeee, 0xfeed }, // 0x648
{ 0xfef0, 0xfeef, 0xfef0, 0xfeef },
{0xfef2, 0xfef3, 0xfef4, 0xfef1 }, // 0x64A
};

// Character set one, used to read the previous connection
#define ARABIC_CHARSET_CNT  23
static const uint16_t Arabic_CharSet1[ARABIC_CHARSET_CNT] = {
0x62c, 0x62d, 0x62e, 0x647, 0x639, 0x63a, 0x641, 0x642,
0x62b, 0x635, 0x636, 0x637, 0x643, 0x645, 0x646, 0x62a,
0x644, 0x628, 0x64a, 0x633, 0x634, 0x638, 0x626
};

// Character set two, used to determine the next continuation
#define ARABIC_CHARSET2_CNT  35
static const uint16_t Arabic_CharSet2[35] = {
0x62c, 0x62d, 0x62e, 0x647, 0x639, 0x63a, 0x641, 0x642,
0x62b, 0x635, 0x636, 0x637, 0x643, 0x645, 0x646, 0x62a,
0x644, 0x628, 0x64a, 0x633, 0x634, 0x638, 0x626,
0x627, 0x623, 0x625, 0x622, 0x62f, 0x630, 0x631, 0x632,
0x648, 0x624, 0x629, 0x649
};

// Special codeword for continuous writing
static const uint16_t Arabic_CharSpecs[][2] = {
{ 0xFEF5, 0xFEF6 },
{ 0xFEF7, 0xFEF8 },
{ 0xFEF9, 0xFEFA },
{ 0xFEFB, 0xFEFC },
};
#endif /* AR_TABLE_TEST_ONLY */

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

#if (AR_TABLE_TEST_ONLY)
bool Arabic_GetLigatrueFirstState(uint16_t now)
{
    int index;
    for(index = 0; index < ARABIC_CHARSET_CNT; index ++){
        if( now == Arabic_CharSet1[index] ) return true;
    }
    return false;
}

bool Arabic_GetLigatureLastState(uint16_t now)
{
    int index;
    for(index = 0; index < ARABIC_CHARSET2_CNT; index ++){
        if( now == Arabic_CharSet2[index] ) return true;
    }
    return false;
}

bool Arabic_GetLigatureSpecCharState(uint16_t now, uint16_t next)
{

    if( now != 0x0644 ) return false;

    if( next == 0x0622 || next == 0x0623 || next == 0x0625 || next == 0x0627 ) return true;

    return false;
}

uint16_t Arabic_GetChar(uint16_t prev, uint16_t now, uint16_t next)
{

    if( now <= 0x0621 || now >= 0x064A ) return now;

    uint32_t offset = now - 0x0621;

    bool first_flag = Arabic_GetLigatrueFirstState(prev);
    bool last_flag = Arabic_GetLigatureLastState(next);

    uint8_t state = (first_flag << 0) | (last_flag << 1);

    uint16_t code;

    switch( state ){
        case 0:
        {
            now = Arabic_PositionTable[offset][3];
        }
        break;
        case 1:
        {
            now = Arabic_PositionTable[offset][0];
        }
        break;
        case 2:
        {
            now = Arabic_PositionTable[offset][1];
        }
        break;
        case 3:
        {
            now = Arabic_PositionTable[offset][2];
        }
        break;
        default:
        {
            now = Arabic_PositionTable[offset][3];
        }
        break;
    }

    return now;
}

uint16_t Arabic_GetLigatureSpecChar(uint16_t prev, uint16_t next)
{

    uint16_t code  = 0;
    uint8_t  state = Arabic_GetLigatrueFirstState(prev);

    switch( next ){
        case 0x0622:
        {
            code = Arabic_CharSpecs[0][state];
        }
        break;
        case 0x0623:
        {
            code = Arabic_CharSpecs[1][state];
        }
        break;
        case 0x0625:
        {
            code = Arabic_CharSpecs[2][state];
        }
        break;
        case 0x0627:
        {
            code = Arabic_CharSpecs[3][state];
        }
        break;
    }

    return code;
}

void arabic_parse(uint16_t *in, uint32_t in_count, uint16_t *out, uint32_t *out_count)
{

    if( in_count == 0 ){
        *out_count = 0;
        return;
    }
    if( in_count == 1 ){
        *out = *in;
        *out_count = 1;
        return;
    }
    uint32_t index = 0;
    uint32_t count = 0;
    while( index < in_count ){
        if( index == 0 ){
            *out = Arabic_GetChar(0, *in, *(in + 1));
            out ++;
            in ++;
            index ++;
            count ++;
        }else if( index == (in_count - 1) ){
            *out = Arabic_GetChar(*(in - 1), *in, 0);
            index ++;
            count ++;
        }else{
            if( Arabic_GetLigatureSpecCharState(*in, *(in + 1)) ){
                *out = Arabic_GetLigatureSpecChar(*(in - 1), *(in + 1));
                out ++;
                in += 2;
                index += 2;
            }else{
                *out = Arabic_GetChar(*(in - 1), *in, *(in + 1));
                out ++;
                in ++;
                index ++;
            }
            count ++;
        }
    }

    *out_count = count;
}
#endif /* AR_TABLE_TEST_ONLY */

#if ((AR_REVERSE_SUPPORT && AR_TABLE_TEST_ONLY) || USE_FRIBIDI)
//count full letter width diff for AR ligatures
lv_coord_t ar_letter_diff(uint32_t start, uint32_t end, const char * txt,
                          uint16_t ar_table_txt[], uint32_t fribidi_txt[],
                          const lv_font_t * font, int cnt)
{
    uint32_t i = start;
#if (AR_REVERSE_SUPPORT && AR_TABLE_TEST_ONLY)
    int index = cnt - 1;
#elif (USE_FRIBIDI)
    int index = 0;
#endif
    lv_coord_t letter_w;
    lv_coord_t letter_origin = 0;
    lv_coord_t letter_ligatures = 0;
    uint32_t letter;
    uint32_t letter_next;

    while(i < end) {
        letter      = lv_txt_encoded_next(txt, &i);
        letter_next = lv_txt_encoded_next(&txt[i], NULL);
        letter_w = lv_font_get_glyph_width(font, letter, letter_next);
        letter_origin += letter_w;
    }
    while(1) {
#if (AR_REVERSE_SUPPORT && AR_TABLE_TEST_ONLY)
        if (index >= 0) {
            letter       = ar_table_txt[index];
            letter_next  = (index == 0) ? 0 : ar_table_txt[index - 1];
            index--;
#elif (USE_FRIBIDI)
        if (index < cnt) {
            letter       = fribidi_txt[index];
            letter_next  = fribidi_txt[index + 1];
            index++;
#endif
            letter_w = lv_font_get_glyph_width(font, letter, letter_next);
            letter_ligatures += letter_w;
        } else {
            break;
        }
    }
    return(letter_origin - letter_ligatures);
}

//adjust AR txt pos.x for ligatures
lv_coord_t adjust_ar_pos(uint32_t start, uint32_t end, const char * origin_txt,
                         uint16_t ar_table_txt[], uint32_t fribidi_txt[],
                         const lv_font_t * font, int cnt, lv_txt_flag_t flag, lv_point_t pos)
{
    if(is_txt_rtl(origin_txt)){
        int x_diff = ar_letter_diff(start, end, origin_txt, ar_table_txt, fribidi_txt, font, cnt);

        //skip AR language adjust in settings language
        if(is_ltr() && (flag == LV_TXT_FLAG_NONE)){
            return pos.x;
        }
        if(flag & LV_TXT_FLAG_RIGHT) {
            //align to right
            pos.x += x_diff;
        } else{
            //treat others to align to center
            pos.x += x_diff/2;
        }
    }
    return pos.x;
}
#endif

/**
 * Write a text
 * @param coords coordinates of the label
 * @param mask the label will be drawn only in this area
 * @param style pointer to a style
 * @param opa_scale scale down all opacities by the factor
 * @param txt 0 terminated text to write
 * @param flag settings for the text from 'txt_flag_t' enum
 * @param offset text offset in x and y direction (NULL if unused)
 * @param sel_start start index of selected area (`LV_LABEL_TXT_SEL_OFF` if none)
 * @param sel_end end index of selected area (`LV_LABEL_TXT_SEL_OFF` if none)
 */
void lv_draw_label(const lv_area_t * coords, const lv_area_t * mask, const lv_style_t * style, lv_opa_t opa_scale,
                   const char * txt, lv_txt_flag_t flag, lv_point_t * offset, uint16_t sel_start, uint16_t sel_end,
                   lv_draw_label_hint_t * hint)
{
    const lv_font_t * font = style->text.font;
    lv_coord_t w;
    if((flag & LV_TXT_FLAG_EXPAND) == 0) {
        /*Normally use the label's width as width*/
        w = lv_area_get_width(coords);
    } else {
        /*If EXAPND is enabled then not limit the text's width to the object's width*/
        lv_point_t p;
        lv_txt_get_size(&p, txt, style->text.font, style->text.letter_space, style->text.line_space, LV_COORD_MAX,
                        flag);
        w = p.x;
    }

    lv_coord_t line_height = lv_font_get_line_height(font) + style->text.line_space;

    /*Init variables for the first line*/
    lv_coord_t line_width = 0;
    lv_point_t pos;
    pos.x = coords->x1;
    pos.y = coords->y1;

    lv_coord_t x_ofs = 0;
    lv_coord_t y_ofs = 0;
    if(offset != NULL) {
        x_ofs = offset->x;
        y_ofs = offset->y;
        pos.y += y_ofs;
    }

    uint32_t line_start     = 0;
    int32_t last_line_start = -1;

    /*Check the hint to use the cached info*/
    if(hint && y_ofs == 0 && coords->y1 < 0) {
        /*If the label changed too much recalculate the hint.*/
        if(LV_MATH_ABS(hint->coord_y - coords->y1) > LV_LABEL_HINT_UPDATE_TH - 2 * line_height) {
            hint->line_start = -1;
        }
        last_line_start = hint->line_start;
    }

    /*Use the hint if it's valid*/
    if(hint && last_line_start >= 0) {
        line_start = last_line_start;
        pos.y += hint->y;
    }

    uint32_t line_end = line_start + lv_txt_get_next_line(&txt[line_start], font, style->text.letter_space, w, flag);

    /*Go the first visible line*/
    while(pos.y + line_height < mask->y1) {
        /*Go to next line*/
        line_start = line_end;
        line_end += lv_txt_get_next_line(&txt[line_start], font, style->text.letter_space, w, flag);
        pos.y += line_height;

        /*Save at the threshold coordinate*/
        if(hint && pos.y >= -LV_LABEL_HINT_UPDATE_TH && hint->line_start < 0) {
            hint->line_start = line_start;
            hint->y          = pos.y - coords->y1;
            hint->coord_y    = coords->y1;
        }

        if(txt[line_start] == '\0') return;
    }

    /*Align to middle*/
    if(flag & LV_TXT_FLAG_CENTER) {
        line_width = lv_txt_get_width(&txt[line_start], line_end - line_start, font, style->text.letter_space, flag);

        pos.x += (lv_area_get_width(coords) - line_width) / 2;

    }
    /*Align to the right*/
    else if(flag & LV_TXT_FLAG_RIGHT) {
        line_width = lv_txt_get_width(&txt[line_start], line_end - line_start, font, style->text.letter_space, flag);
        pos.x += lv_area_get_width(coords) - line_width;
    }

    lv_opa_t opa = opa_scale == LV_OPA_COVER ? style->text.opa : (uint16_t)((uint16_t)style->text.opa * opa_scale) >> 8;

    cmd_state_t cmd_state = CMD_STATE_WAIT;
    uint32_t i;
    uint16_t par_start = 0;
    lv_color_t recolor;
    lv_coord_t letter_w;
    lv_style_t sel_style;
    lv_style_copy(&sel_style, &lv_style_plain_color);
    sel_style.body.main_color = sel_style.body.grad_color = style->text.sel_color;

    /*Write out all lines*/
    while(txt[line_start] != '\0') {
        if(offset != NULL) {
            pos.x += x_ofs;
        }
        /*Write all letter of a line*/
        cmd_state = CMD_STATE_WAIT;
        i         = line_start;
        uint32_t letter;
        uint32_t letter_next;

#if (AR_REVERSE_SUPPORT)
        //[ZX53]temp solution to reverse all text for AR language start
        //TODO need to implement bidi for AR later
        bool reverse_line = false;

        bool non_rtl_start_b = false; // flag to check if finish parsing txt starts with non rtl letters
        uint16_t start_cnt = 0; // num of non rtl letters at txt starting point
        uint16_t start_num = 0; // variable to storage how many non rtl letters left to handle
        uint32_t start_idx = line_start;
        bool non_rtl_end_b = false; // flag to check if finish parsing txt ending with non rtl letters
        uint16_t end_cnt = 0; // num of non rtl letters at txt ending point
        uint16_t end_num = 0; // variable to storage how many non rtl letters left to handle
        uint32_t end_idx = line_end;
        uint16_t non_rtl_cnt = 0, count = 0; // used to check how many non rtl letters left to handle in middle of txt
#if (AR_TABLE_TEST_ONLY)
        uint16_t arabic_txt[line_end];
        memset(arabic_txt, 0, sizeof(arabic_txt));
        uint32_t arabic_txt_cnt = 0;
#endif
        if(is_txt_rtl(txt)){
            //reverse line if contain AR language in line text
            reverse_line = true;

            // count no. of non rtl at txt begining and ending
            start_cnt = start_num = count_non_rtl_begin(txt, line_start);
            end_cnt = end_num = count_non_rtl_ending(txt, line_start, line_end);
#if (AR_TABLE_TEST_ONLY)
            uint32_t i = line_start, j = 0;
            uint32_t letter;
            while(i < line_end) {
                letter = lv_txt_encoded_next(txt, &i);
                if (letter != 0x0) {
                    arabic_txt[j] = letter;
                }
                j++;
                arabic_txt[j] = 0;
            }
            arabic_txt_cnt = j;
#endif
        }

#if (AR_REVERSE_SUPPORT && AR_TABLE_TEST_ONLY)
        uint16_t arabic_txt_output[arabic_txt_cnt + 1];
        uint32_t arabic_output_cnt = 0;
        if (reverse_line) {
            memset(arabic_txt_output, 0, sizeof(arabic_txt_output));
            arabic_parse(arabic_txt, arabic_txt_cnt, arabic_txt_output, &arabic_output_cnt);
            pos.x = adjust_ar_pos(line_start, line_end, txt,
                    arabic_txt_output, NULL, font, arabic_output_cnt, flag, pos);
        }
        int index = arabic_output_cnt - 1;
        while(i < line_end) {
            if (reverse_line) {
                if (index >= 0) {
                    letter       = arabic_txt_output[index];
                    letter_next  = (index == 0) ? 0 : arabic_txt_output[index - 1];
                    index--;
                } else {
                    break;
                }
            } else {
                letter      = lv_txt_encoded_next(txt, &i);
                letter_next = lv_txt_encoded_next(&txt[i], NULL);
            }
#else /* AR_REVERSE_SUPPORT && AR_TABLE_TEST_ONLY */
        reverse_line ? (i = line_end) : (i = line_start);
        while(reverse_line ? (i > line_start + start_cnt) : (i < line_end + end_num)) {
            if(reverse_line){
                // parsing txt starts with non rtl letters
                if (start_num >= 0) {
                    if (start_num == 0) {
                        non_rtl_start_b = true;
                    } else {
                        letter = lv_txt_encoded_next(txt, &start_idx);
                        start_num--;
                    }
                }
                // update i to skip parsing txt ending with non rtl letters and handle when finish parsing line
                if (i == line_end && end_cnt >= 0 && !non_rtl_end_b) {
                    i = line_end - end_cnt;
                }

                // parse encoded previous, if found letter is non rtl, count number of rtl for further handle
                if (non_rtl_cnt == 0 && non_rtl_start_b) {
                    letter = lv_txt_encoded_prev(txt, &i);
                    if (!is_letter_rtl(letter)) {
                        uint32_t k, l = i + 1;
                        for(k = i + 1; k > line_start; k--) {
                            uint32_t letter_prev = lv_txt_encoded_prev(txt, &l);
                            if (is_letter_rtl(letter_prev))
                                break;
                            non_rtl_cnt++;
                        }
                        if (non_rtl_cnt > 1) {
                            count = non_rtl_cnt;
                            i = line_end - (line_end - i) - non_rtl_cnt + 1;
                        } else {
                            non_rtl_cnt = 0;
                        }
                    }
                }

                // parse non rtl letters
                if (non_rtl_cnt > 0 && non_rtl_start_b) {
                    letter = lv_txt_encoded_next(txt, &i);
                    non_rtl_cnt--;
                    if (non_rtl_cnt == 0) {
                        i = i - count;
                    }
                }

                // parsing txt ending with non rtl letters
                if (i == (line_start + start_cnt)) {
                    non_rtl_end_b = true;
                }
                if (non_rtl_end_b) {
                    if (end_num == 0) {
                        non_rtl_end_b = false;
                        i = line_start;
                    } else {
                        if (end_cnt == end_num) {
                            i = line_end;
                        }
                        end_num--;
                    }
                }
            }else{
                letter = lv_txt_encoded_next(txt, &i);
            }
            //[ZX53]temp solution to reverse all text for AR language end
            letter_next = lv_txt_encoded_next(&txt[i], NULL);
#endif /* AR_REVERSE_SUPPORT && AR_TABLE_TEST_ONLY */
#else /* AR_REVERSE_SUPPORT */
#if (USE_FRIBIDI)
        bool reverse_line = false;
        uint32_t fribidi_txt[line_end];
        memset(fribidi_txt, 0, sizeof(fribidi_txt));
        int fribidi_txt_cnt = 0;

        if(is_txt_rtl(txt)){
            //reverse line if contain AR language in line text
            reverse_line = is_txt_line_rtl(txt, line_start, line_end);

            // count no. of non rtl at txt begining and ending
            uint16_t start_cnt = count_non_rtl_begin(txt, line_start);
            if (skip_non_rtl_reappend(txt, line_start)) start_cnt = 0;

            uint32_t i = line_start + start_cnt, j = 0;
            uint32_t letter;
            while(i < line_end) {
                letter = lv_txt_encoded_next(txt, &i);
                if (letter != 0x0) {
                    fribidi_txt[j] = letter;
                }
                j++;
                fribidi_txt[j] = 0;
            }
            // append non rtl letters back to end of txt
            uint32_t k, m = line_start;
            for(k = 0; k < start_cnt; k++) {
                letter = lv_txt_encoded_next(txt, &m);
                fribidi_txt[j] = letter;
                j++;
                fribidi_txt[j] = 0;
            }

            fribidi_txt_cnt = j;
        }

        uint32_t fribidi_visual_str[fribidi_txt_cnt + 1];
        if (reverse_line) {
            memset(fribidi_visual_str, 0, sizeof(fribidi_visual_str));
            uint32_t pbase_dir = FRIBIDI_TYPE_RTL;
            int stat = fribidi_log2vis(fribidi_txt, fribidi_txt_cnt, &pbase_dir,
                            fribidi_visual_str, NULL, NULL, NULL);
            pos.x = adjust_ar_pos(line_start, line_end, txt,
                            NULL, fribidi_visual_str, font, fribidi_txt_cnt, flag, pos);
        }
        int index = 0;
        while(i < line_end) {
            if (reverse_line) {
                if (index < fribidi_txt_cnt) {
                    letter       = fribidi_visual_str[index];
                    letter_next  = fribidi_visual_str[index + 1];
                    index++;
                } else {
                    break;
                }
            } else {
                letter      = lv_txt_encoded_next(txt, &i);
                letter_next = lv_txt_encoded_next(&txt[i], NULL);
            }
#else
        while(i < line_end) {
            letter      = lv_txt_encoded_next(txt, &i);
            letter_next = lv_txt_encoded_next(&txt[i], NULL);
#endif /* USE_FRIBIDI */
#endif /* AR_REVERSE_SUPPORT */

            /*Handle the re-color command*/
            if((flag & LV_TXT_FLAG_RECOLOR) != 0) {
                if(letter == (uint32_t)LV_TXT_COLOR_CMD[0]) {
                    if(cmd_state == CMD_STATE_WAIT) { /*Start char*/
                        par_start = i;
                        cmd_state = CMD_STATE_PAR;
                        continue;
                    } else if(cmd_state == CMD_STATE_PAR) { /*Other start char in parameter escaped cmd. char */
                        cmd_state = CMD_STATE_WAIT;
                    } else if(cmd_state == CMD_STATE_IN) { /*Command end */
                        cmd_state = CMD_STATE_WAIT;
                        continue;
                    }
                }

                /*Skip the color parameter and wait the space after it*/
                if(cmd_state == CMD_STATE_PAR) {
                    if(letter == ' ') {
                        /*Get the parameter*/
                        if(i - par_start == LABEL_RECOLOR_PAR_LENGTH + 1) {
                            char buf[LABEL_RECOLOR_PAR_LENGTH + 1];
                            memcpy(buf, &txt[par_start], LABEL_RECOLOR_PAR_LENGTH);
                            buf[LABEL_RECOLOR_PAR_LENGTH] = '\0';
                            int r, g, b;
                            r       = (hex_char_to_num(buf[0]) << 4) + hex_char_to_num(buf[1]);
                            g       = (hex_char_to_num(buf[2]) << 4) + hex_char_to_num(buf[3]);
                            b       = (hex_char_to_num(buf[4]) << 4) + hex_char_to_num(buf[5]);
                            recolor = lv_color_make(r, g, b);
                        } else {
                            recolor.full = style->text.color.full;
                        }
                        cmd_state = CMD_STATE_IN; /*After the parameter the text is in the command*/
                    }
                    continue;
                }
            }

            lv_color_t color = style->text.color;

            if(cmd_state == CMD_STATE_IN) color = recolor;

            letter_w = lv_font_get_glyph_width(font, letter, letter_next);

            if(sel_start != 0xFFFF && sel_end != 0xFFFF) {
                int char_ind = lv_encoded_get_char_id(txt, i);
                /*Do not draw the rectangle on the character at `sel_start`.*/
                if(char_ind > sel_start && char_ind <= sel_end) {
                    lv_area_t sel_coords;
                    sel_coords.x1 = pos.x;
                    sel_coords.y1 = pos.y;
                    sel_coords.x2 = pos.x + letter_w + style->text.letter_space - 1;
                    sel_coords.y2 = pos.y + line_height - 1;
                    lv_draw_rect(&sel_coords, mask, &sel_style, opa);
                }
            }
            lv_draw_letter(&pos, mask, font, letter, color, opa);

            if(letter_w > 0) {
                pos.x += letter_w + style->text.letter_space;
            }
        }
        /*Go to next line*/
        line_start = line_end;
        line_end += lv_txt_get_next_line(&txt[line_start], font, style->text.letter_space, w, flag);

        pos.x = coords->x1;
        /*Align to middle*/
        if(flag & LV_TXT_FLAG_CENTER) {
            line_width =
                lv_txt_get_width(&txt[line_start], line_end - line_start, font, style->text.letter_space, flag);

            pos.x += (lv_area_get_width(coords) - line_width) / 2;

        }
        /*Align to the right*/
        else if(flag & LV_TXT_FLAG_RIGHT) {
            line_width =
                lv_txt_get_width(&txt[line_start], line_end - line_start, font, style->text.letter_space, flag);
            pos.x += lv_area_get_width(coords) - line_width;
        }

        /*Go the next line position*/
        pos.y += line_height;

        if(pos.y > mask->y2) return;
    }
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/**
 * Convert a hexadecimal characters to a number (0..15)
 * @param hex Pointer to a hexadecimal character (0..9, A..F)
 * @return the numerical value of `hex` or 0 on error
 */
static uint8_t hex_char_to_num(char hex)
{
    uint8_t result = 0;

    if(hex >= '0' && hex <= '9') {
        result = hex - '0';
    } else {
        if(hex >= 'a') hex -= 'a' - 'A'; /*Convert to upper case*/

        switch(hex) {
            case 'A': result = 10; break;
            case 'B': result = 11; break;
            case 'C': result = 12; break;
            case 'D': result = 13; break;
            case 'E': result = 14; break;
            case 'F': result = 15; break;
            default: result = 0; break;
        }
    }

    return result;
}

static bool skip_non_rtl_reappend(const char * txt, uint32_t line_start)
{
    uint32_t i = line_start, j = 0;
    uint32_t letter;
    while(txt[i] != '\0') {
        letter = lv_txt_encoded_next(txt, &i);
        if (letter == 0x0028) {   // ( left parenthesis, for SSID Security WPA2 AES
            return true;
        }
    }
    return false;
}

static uint32_t count_non_rtl_begin(const char * txt, uint32_t line_start)
{
    uint32_t i = line_start, j = 0;
    uint32_t letter;
    while(txt[i] != '\0') {
        letter = lv_txt_encoded_next(txt, &i);

        if (is_letter_rtl(letter)) {
            return j;
        }
        j++;
    }
}

static uint32_t count_non_rtl_ending(const char * txt, uint32_t line_start, uint32_t line_end)
{
    uint32_t i = line_end, j = 0;
    uint32_t letter;
    while(i > line_start) {
        letter = lv_txt_encoded_prev(txt, &i);

        if (is_letter_rtl(letter)) {
            return j;
        }
        j++;
    }
}
