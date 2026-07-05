#ifndef CACHE_H
#define CACHE_H

#include <QString>
#include "vedis.h"

class Cache
{
    public:
        static Cache& instance();

        explicit Cache(const QString& path);
        ~Cache();

        bool isOpen() const;
        void transaction();
        void flush();

        void set(const QString& key, const QString& value);
        void set(const QString& key, bool value);
        void set(const QString& key, int value);

        QString get(const QString& key, const QString& defaultValue = {}) const;
        bool getBool(const QString& key, bool defaultValue = false) const;
        int getInt(const QString& key, int defaultValue = 0) const;

    private:
        vedis* m_db = nullptr;

        QString fetch(const QString& key) const;
};

#endif // CACHE_H
