/*
 * JsonItem.cpp
 *
 *  Created on: 09/11/2014
 *      Author: Victor
 */
#include "JsonItem.hpp"
#include <bb/cascades/Container>
#include <bb/cascades/DockLayout>
#include <bb/cascades/Label>
#include <bb/cascades/StackLayout>
#include <bb/cascades/SystemDefaults>
#include <bb/cascades/TextStyle>
using namespace bb::cascades;

JsonItem::JsonItem(Container *parent) :
        CustomListItem(HighlightAppearance::None, parent)
{
    Container *itemContainer = Container::create().layout(DockLayout::create())
    .horizontal(HorizontalAlignment::Fill).vertical(VerticalAlignment::Fill);

    mHighlightContainer = Container::create().background(Color::fromARGB(0xff75b5d3))
                                .horizontal(HorizontalAlignment::Fill).vertical(VerticalAlignment::Fill)
                                .opacity(0.0);
    Container *contentContainer = new Container();
    contentContainer->setLayout(StackLayout::create().orientation(LayoutOrientation::LeftToRight));
    contentContainer->setImplicitLayoutAnimationsEnabled(false);

    mItemLabel = Label::create();
    mItemLabel->setVerticalAlignment(VerticalAlignment::Center);
    mItemLabel->textStyle()->setBase(SystemDefaults::TextStyles::titleText());
    mItemLabel->textStyle()->setColor(Color::Black);
    mItemLabel->setImplicitLayoutAnimationsEnabled(false);

    contentContainer->add(mItemLabel);
    itemContainer->add(mHighlightContainer);
    itemContainer->add(contentContainer);

    setDividerVisible(false);
    setContent(itemContainer);

}

void JsonItem::updateItem(const QString text)
{
    // Update image and text for the current item
    mItemLabel->setText(text);
}

void JsonItem::select(bool select)
{
    // When an item is selected, show the colored highlight Container
    if (select) {
        mHighlightContainer->setOpacity(0.9f);
    } else {
        mHighlightContainer->setOpacity(0.0f);
    }
}

void JsonItem::reset(bool selected, bool activated)
{
    Q_UNUSED(activated);

    // Since items are recycled, the reset function is where we have
    // to make sure that item state (defined by the arguments) is correct.
    select(selected);
}

void JsonItem::activate(bool activate)
{
    // There is no special activate state; selected and activated look the same.
    select(activate);
}

