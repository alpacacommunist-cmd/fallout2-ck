#ifndef CK_I18N_H
#define CK_I18N_H

#include <string>

namespace ck::i18n {
    void load_language(const std::string& language);

    const char* t(const std::string& key);
}

#endif
