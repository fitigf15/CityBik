/*
 * JsonItemProvider.hpp
 *
 *  Created on: 09/11/2014
 *      Author: Victor
 */

#ifndef JSONITEMPROVIDER_HPP_
#define JSONITEMPROVIDER_HPP_

#include <bb/cascades/ListItemProvider>
using namespace bb::cascades;

class JsonItemProvider: public ListItemProvider
{
    Q_OBJECT
    public:
        JsonItemProvider(QObject *parent=0);
        virtual ~JsonItemProvider();

        VisualNode * createItem(ListView* list, const QString &type);

        void updateItem(ListView* list, VisualNode *listItem, const QString &type,
                    const QVariantList &indexPath, const QVariant &data);

};

#endif /* JSONITEMPROVIDER_HPP_ */
