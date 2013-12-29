#ifndef _ML_LEVEL_COLLECTION_EDITEE_H
#define _ML_LEVEL_COLLECTION_EDITEE_H

#include <sigc++/sigc++.h>

namespace sigc {
SIGC_FUNCTORS_DEDUCE_RESULT_TYPE_WITH_DECLTYPE
}

#include <io/LevelData.hpp>

class LevelCollectionEditee;

typedef sigc::signal<void, LevelCollectionEditee*> LevelCollectionNotifyEvent;
typedef sigc::signal<void, LevelCollectionEditee*, const SharedLevel&> LevelCollectionLevelEvent;

class LevelCollectionEditee
{
public:
    LevelCollectionEditee(const SharedLevelCollection &editee, const std::string &name);
    LevelCollectionEditee(const LevelCollectionEditee &ref) = delete;
    LevelCollectionEditee& operator=(const LevelCollectionEditee &ref) = delete;

private:
    std::string _name;
    SharedLevelCollection _editee;
    LevelCollectionNotifyEvent _changed;
    LevelCollectionLevelEvent _level_changed;
    LevelCollectionLevelEvent _level_created;
    LevelCollectionLevelEvent _level_deleted;

protected:
    void changed();
    void level_created(const SharedLevel &level);
    void level_deleted(const SharedLevel &level);

public:
    std::vector<SharedLevel> &levels();

    const SharedLevelCollection &editee() const {
        return _editee;
    };

    const std::string &get_name() const {
        return _name;
    };

public:
    SharedLevel add_level(const SharedLevel &level);
    void delete_level(const SharedLevel &level);
    SharedLevel new_level(const std::string &unique_name);

public:
    void set_name(const std::string &name);

public:
    void set_author(const std::string &author);
    void set_description(const std::string &description);
    void set_display_name(const std::string &display_name);
    void set_license(const std::string &license);
    void set_version(const std::string &version);

public:
    void level_changed(const SharedLevel &level);

public:
    inline LevelCollectionNotifyEvent signal_changed() {
        return _changed;
    };

    inline LevelCollectionLevelEvent signal_level_changed() {
        return _level_changed;
    };

    inline LevelCollectionLevelEvent signal_level_created() {
        return _level_created;
    };

    inline LevelCollectionLevelEvent signal_level_deleted() {
        return _level_deleted;
    };

};

typedef std::unique_ptr<LevelCollectionEditee> LevelCollectionEditeePtr;

#endif
