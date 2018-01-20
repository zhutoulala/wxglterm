#include "freetype_gl.h"

#include <string.h>
#include <iostream>
#include <fontconfig/fontconfig.h>
#include <sstream>

#define SINGLE_WIDTH_CHARACTERS         \
					" !\"#$%&'()*+,-./" \
					"0123456789" \
					":;<=>?@" \
					"ABCDEFGHIJKLMNOPQRSTUVWXYZ" \
					"[\\]^_`" \
					"abcdefghijklmnopqrstuvwxyz" \
					"{|}~"

freetype_gl_context_ptr freetype_gl_init() {
    freetype_gl_context_ptr ptr = std::make_shared<freetype_gl_context>();

    return ptr;
}

freetype_gl_context::freetype_gl_context()
    : font_manager {ftgl::font_manager_new(512, 512, 1)}
    , font_name {"Mono"}
    , font_size {16} {
        memset(fonts_markup, 0, sizeof(fonts_markup));
      }

freetype_gl_context::~freetype_gl_context() {
    cleanup();

    ftgl::font_manager_delete(font_manager);
}

void freetype_gl_context::cleanup() {
    for(int i=0;i < FontCategoryEnum::FontCategoryCount;i++) {
        if (fonts_markup[i].family)
            free(fonts_markup[i].family);

        memset(&fonts_markup[i], 0, sizeof(ftgl::markup_t));
    }
}

static
char *
match_description(const char * description )
{

#if (defined(_WIN32) || defined(_WIN64)) && !defined(__MINGW32__)
    fprintf( stderr, "\"font_manager_match_description\" "
                     "not implemented for windows.\n" );
    return 0;
#endif

    char *filename = 0;
    FcInit();
    FcPattern *pattern = FcNameParse((const FcChar8 *)description);
    FcConfigSubstitute( 0, pattern, FcMatchPattern );
    FcDefaultSubstitute( pattern );
    FcResult result;
    FcPattern *match = FcFontMatch( 0, pattern, &result );
    FcPatternDestroy( pattern );

    if ( !match )
    {
        fprintf( stderr, "fontconfig error: could not match description '%s'", description );
        return 0;
    }
    else
    {
        FcValue value;
        FcResult result = FcPatternGet( match, FC_FILE, 0, &value );
        if ( result )
        {
            fprintf( stderr, "fontconfig error: could not match description '%s'", description );
        }
        else
        {
            filename = strdup( (char *)(value.u.s) );
        }
    }
    FcPatternDestroy( match );
    return filename;
}

ftgl::texture_font_t * freetype_gl_context::get_font(FontCategoryEnum font_category) {
    if (fonts_markup[font_category].font)
        return fonts_markup[font_category].font;

    std::stringstream ss;
    ss << font_name << ":size=" << font_size;

    fonts_markup[font_category].family = match_description(ss.str().c_str());
    fonts_markup[font_category].size = (float)font_size;
    fonts_markup[font_category].bold = (font_category == FontCategoryEnum::Bold || font_category == FontCategoryEnum::BoldUnderlined);
    fonts_markup[font_category].underline = (font_category == FontCategoryEnum::Underlined || font_category == FontCategoryEnum::BoldUnderlined);

    fonts_markup[font_category].font =
            font_manager_get_from_markup(font_manager,
                                         &fonts_markup[font_category]);


    return fonts_markup[font_category].font;
}

void freetype_gl_context::init_font(const std::string & name, uint64_t size) {
    this->font_name = name;
    this->font_size = size;

    cleanup();

    ftgl::texture_font_t * f = get_font(FontCategoryEnum::Default);

    float line_height = font_size, col_width = 0.0f;

    (void)f;
    for(size_t i=0;i < strlen(SINGLE_WIDTH_CHARACTERS); i++) {
        ftgl::texture_glyph_t *glyph =
                texture_font_get_glyph(f,
                                       &SINGLE_WIDTH_CHARACTERS[i]);
        if( glyph != NULL ) {
            if (line_height < glyph->advance_y)
                line_height = glyph->advance_y;

            if (col_width < glyph->advance_x) {
                col_width = glyph->advance_x;
            }
        }
    }

    std::cout << "height:" << line_height << ", width:" << col_width << std::endl;
}
