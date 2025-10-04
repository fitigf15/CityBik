/*
 * JsonItem.hpp
 *
 *  Created on: 09/11/2014
 *      Author: Victor
 */

#ifndef _JSONITEM_HPP_
#define _JSONITEM_HPP_

#include <bb/cascades/CustomListItem>
#include <bb/cascades/ListItemListener>
using namespace bb::cascades;

namespace bb
{
    namespace cascades
    {
        class Label;
        class Container;
    }
}

class JsonItem: public bb::cascades::CustomListItem, public ListItemListener
{
    Q_OBJECT

public:

    JsonItem(Container *parent = 0);

    void updateItem(const QString text);
    void select(bool select);
    void reset(bool selected, bool activated);
    void activate(bool activate);

private:

    Label *mItemLabel;
    Container *mHighlightContainer;

};




#endif /* JSONITEM_HPP_ */
