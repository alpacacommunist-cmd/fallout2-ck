#ifndef CK_PROTO_CACHE_H
#define CK_PROTO_CACHE_H

#include <string>
#include <vector>

// Структура данных (остается без изменений, добавили только description)
struct CkProtoInfo {
    int pid = -1;
    int fid = -1;
    std::string name;
    std::string filename;
    std::string description; // Новое полезное поле для будущего GUI!
    int type = -1;           // OBJ_TYPE_*
};

// Вперед-объявление структуры sqlite3, чтобы не тянуть тяжелый хедер в заголовочный файл
struct sqlite3;

class CkProtoCache {
public:
    CkProtoCache();
    ~CkProtoCache();

    // Инициализирует базу данных из файла. Если файла нет — пересоздает и парсит движок.
    bool initialize(const std::string& cachePath);

    // Поиск прототипа по имени (например, "tstcv")
    // Возвращает объект по значению (или пустой объект, если не найдено)
    CkProtoInfo getByName(const std::string& name, int type = -1) const;

    // Поиск прототипа по PID
    CkProtoInfo getByPid(int pid) const;

    // Получить список всех прототипов определенного типа (например, всех криттеров)
    std::vector<CkProtoInfo> getByType(int type) const;

private:
    sqlite3* db = nullptr; // Наш указатель на открытую БД SQLite

    bool buildFromEngine(const std::string& cachePath);
    bool createTables();
};

extern CkProtoCache gProtoCache;
#endif
