/**
 * @brief Menu backend with rendering and scrolling support.
 *
 * @author https://github.com/Konajka
 * @version 0.1 2019-05-01
 *  Base menu model definition.
 * @version 0.2 2019-05-03
 *  Base rendering.
 * @version 0.3 2019-05-05
 *  Menu items scrolling support for rendering.
 * @version 0.4 2019-05-14
 *  Added doc comments.
 * @version 0.5 2019-05-15
 *  Added controller events.
 *  Separated renderer classes.
 * @version 0.6 2019-05-17
 *  Added menu index to renderer.
 *  Added user custom tag and data pointer definition.
 * @version 0.7 2019-05-20
 *  Fixed active menu item changing to change before event is yield.
 */

#ifndef QMENU_H
#define QMENU_H

#include <Arduino.h>
#include <String.h>

// Undefine to lower memory consumption
#define QMENU_USE_EXTENDED_INFO

/**
 * @brief Menu item definition.
 */
class QMenuItem {
    private:
        int _id;
        char* _caption;
        #ifdef QMENU_USE_EXTENDED_INFO
        int _tag;
        void* _data;
        #endif
        QMenuItem* _back = NULL;
        QMenuItem* _menu = NULL;
        QMenuItem* _prev = NULL;
        QMenuItem* _next = NULL;

    public:

        /**
         * @brief Static QMenuItem instantiating and initialization.
         * @param id Menu item identification. This could be unique integer value, but no unique
         *      test is performed.
         * @param caption Menu item caption.
         */
        static QMenuItem* create(int id, char* caption) {
            return new QMenuItem(id, caption);
        }

        /**
         * @brief Static QMenuItem instantiating and initialization.
         * @param id Menu item identification. This could be unique integer value, but no unique
         *      test is performed.
         * @param caption Menu item caption.
         * @param tag User defined integer value.
         */
        static QMenuItem* create(int id, char* caption, int tag) {
            return new QMenuItem(id, caption);
        }

        /**
         * @brief Static QMenuItem instantiating and initialization.
         * @param id Menu item identification. This could be unique integer value, but no unique
         *      test is performed.
         * @param caption Menu item caption.
         * @param data User defined data pointer.
         */
        static QMenuItem* create(int id, char* caption, void* data) {
            return new QMenuItem(id, caption);
        }

        /**
         * @brief Static QMenuItem instantiating and initialization.
         * @param id Menu item identification. This could be unique integer value, but no unique
         *      test is performed.
         * @param caption Menu item caption.
         * @param tag User defined integer value.
         * @param data User defined data pointer.
         */
        static QMenuItem* create(int id, char* caption, int tag, void* data) {
            return new QMenuItem(id, caption);
        }

        /**
         * @brief Creates new QMenuItem instance.
         * @param id Menu item identification.
         * @param caption Menu item caption.
         */
        QMenuItem(int id, char* caption) {
            _id = id;
            _caption = caption;
        }

        #ifdef QMENU_USE_EXTENDED_INFO
        /**
         * @brief Creates new QMenuItem instance.
         * @param id Menu item identification.
         * @param caption Menu item caption.
         * @param tag User defined integer value.
         */
        QMenuItem(int id, char* caption, int tag) {
            _id = id;
            _caption = caption;
            _tag = tag;
        }

        /**
         * @brief Creates new QMenuItem instance.
         * @param id Menu item identification.
         * @param caption Menu item caption.
         * @param data User defined data pointer.
         */
        QMenuItem(int id, char* caption, void* data) {
            _id = id;
            _caption = caption;
            _data = data;
        }

        /**
         * @brief Creates new QMenuItem instance.
         * @param id Menu item identification.
         * @param caption Menu item caption.
         * @param tag User defined integer value.
         * @param data User defined data pointer.
         */
        QMenuItem(int id, char* caption, int tag, void* data) {
            _id = id;
            _caption = caption;
            _tag = tag;
            _data = data;
        }
        #endif

        /**
         * @brief Gets menu item identification.
         * @return Returns menu item identification.
         */
        int getId() {
            return this->_id;
        }

        /**
         * @brief Gets menu item caption.
         * @return Returns menu item caption.
         */
        char* getCaption() {
            return this->_caption;
        }

        #ifdef QMENU_USE_EXTENDED_INFO
        /**
         * @brief Gets user custom integer value.
         * @return Returns tag value.
         */
        int getTag() {
            return _tag;
        }

        /**
         * @brief Sets user custom integer value.
         */
        void setTag(int tag) {
            _tag = tag;
        }

        /**
         * @brief Gets user defined data pointer.
         * @return Returns void pointer to custom data.
         */
        void* getData() {
            return _data;
        }

        /**
         * @brief Sets user defined data pointer.
         */
        void setData(void* data) {
            _data = data;
        }
        #endif

        /**
         * @brief Gets parent menu item.
         * @return Returns parent menu item reference or NULL if this item is top menu item.
         */
        QMenuItem* getBack() {
            return this->_back;
        }

        /**
         * @brief Gets first submenu item.
         * @return Returns first menu item reference from submenu or NULL if this item has no
         *      submenu.
         */
        QMenuItem* getMenu() {
            return this->_menu;
        }

        /**
         * @brief Sets first submenu item.
         * @param menu Reference to menu item that has to be first submenu item. Value of NULL
         * clears submenu.
         */
        QMenuItem* setMenu(QMenuItem* menu) {
            this->_menu = menu;
            menu->_back = this;
            return menu;
        }

        /**
         * @brief Get previous menu item in current menu level.
         * @return Returns previous item in current menu level or NULL if this item is first.
         */
        QMenuItem* getPrev() {
            return this->_prev;
        }

        /**
         * @brief Get next menu item in current menu level.
         * @return Returns next item in current menu level or NULL if this item is last.
         */
        QMenuItem* getNext() {
            return this->_next;
        }

        /**
         * @brief Sets next item in durrent menu level.
         * @param menu Reference to menu item that has to be next item after this item in current
         * menu level.
         */
        QMenuItem* setNext(QMenuItem* next) {
            this->_next = next;
            next->_prev = this;
            next->_back = this->_back;
            return next;
        }
};

/** QMenu on onActiveItemChanged event data */
struct QMenuActiveItemChangedEvent {
    const QMenuItem* oldActiveItem;
    const QMenuItem* newActiveItem;
};

/** QMenu on onActiveItemChanged event callback */
typedef void (*QMenuActiveItemChangedCallback) (QMenuActiveItemChangedEvent);

/** QMenu on onItemUtilized event data */
struct QMenuItemUtilizedEvent {
    const QMenuItem* utilizedItem;
};

/** QMenu on onItemUtilized event callback */
typedef void (*QMenuItemUtilizedCallback) (QMenuItemUtilizedEvent);

/**
 * @brief Menu backend controller.
 */
class QMenu {
    private:
        // Root item
        QMenuItem* _root;
        // Curretly active item
        QMenuItem* _active;

        // Events
        QMenuActiveItemChangedCallback _onActiveItemChanged = NULL;
        QMenuItemUtilizedCallback _onItemUtilized = NULL;

    protected:

        /**
         * @brief Calls onActiveItemChanged event if assigned.
         * @param oldItem Previusly active menu item.
         * @oaram newItem Currently active menu item.
         */
        void doOnActiveItemChanged(QMenuItem* oldItem, QMenuItem* newItem) {
            if (_onActiveItemChanged != NULL) {
                QMenuActiveItemChangedEvent event = {
                    oldItem,
                    newItem
                };
                _onActiveItemChanged(event);
            }
        }

        /**
         * @brief Calls onItemUtilized event if assigned.
         * @param item Utilized item.
         */
        void doOnItemUtilized(QMenuItem* item) {
            if (_onItemUtilized != NULL) {
                QMenuItemUtilizedEvent event = {
                    item
                };
                _onItemUtilized(event);
            }
        }

    public:
        /**
         * @brief Creates new instance and initializes it with default root menu item. Root item is
         *      set as active menu item.
         */
        QMenu() {
            _root = new QMenuItem(0, "__ROOT__");
            _active = _root;
        }

        /**
         * @brief Creates new instance and initializes it with root menu item.
         * @param id Root menu item identification. This could be unique integer value, but no
         *      unique test is performed. Root item is set as active menu item.
         * @param caption Root menu item caption.
         */
        QMenu(int id, char* caption) {
            _root = new QMenuItem(id, caption);
            _active = _root;
        }

        /**
         * @brief Gets root menu item.
         * @return Returns menu root item reference. This is never NULL.
         */
        QMenuItem* getRoot() {
            return _root;
        }

        /**
         * @brief Gets currently active menu item.
         * @return Returns active menu item reference.
         */
        QMenuItem* getActive() {
            return _active;
        }

        /**
         * @brief Gets onActiveItemChanged callback.
         * @return Returns onActiveItemChanged callback or NULL if not assigned.
         */
        QMenuActiveItemChangedCallback getOnActiveItemChanged(){
            return _onActiveItemChanged;
        }

        /**
         * @brief Sets onActiveItemChanged callback.
         * @param onActiveItemChanged The onActiveItemChanged callback.
         */
        void setOnActiveItemChanged(QMenuActiveItemChangedCallback onActiveItemChanged) {
            _onActiveItemChanged = onActiveItemChanged;
        }

        /**
         * @brief Gets onItemUtilized callback.
         * @return Returns onItemUtilized callback or NULL if not assigned.
         */
        QMenuItemUtilizedCallback getOnItemUtilized() {
            return _onItemUtilized;
        }

        /**
         * @brief Sets onItemUtilized callback.
         * @param onItemUtilized The onItemUtilized callback.
         */
        void setOnItemUtilized(QMenuItemUtilizedCallback onItemUtilized) {
            _onItemUtilized = onItemUtilized;
        }

        /**
         * @brief Moves to next menu item. Sets currently active item's next item as active menu
         *      item.
         * @return Returns next menu item reference or NULL if there is no next item.
         */
        QMenuItem* next() {
            if (this->_active != NULL) {
                QMenuItem* oldActive = this->_active;
                QMenuItem* newActive = this->_active->getNext();
                if (newActive != NULL) {
                    this->_active = newActive;
                    doOnActiveItemChanged(oldActive, newActive);
                }
                return newActive;
            }

            return NULL;
        }

        /**
         * @brief Moves to previous menu item. Sets currently active item's previous item as active
         *      menu item.
         * @return Returns previous menu item reference or NULL if there is no previous item.
         */
        QMenuItem* prev() {
            if (this->_active != NULL) {
                QMenuItem* oldActive = this->_active;
                QMenuItem* newActive = this->_active->getPrev();      
                if (newActive != NULL) {
                    this->_active = newActive;
                    doOnActiveItemChanged(oldActive, newActive);
                }
                return newActive;
            }

            return NULL;
        }

        /**
         * @brief Moves to submenu. Sets currently active item's first child item as active menu
         *      item.
         * @return Returns first submenu item reference or NULL if there is no submenu.
         */
        QMenuItem* enter() {
            if (this->_active != NULL) {
                QMenuItem* oldActive = this->_active;
                QMenuItem* newActive = this->_active->getMenu();
                if (newActive != NULL) {
                    this->_active = newActive;
                    doOnActiveItemChanged(oldActive, newActive);
                } else {
                    doOnItemUtilized(this->_active);
                }
                return newActive;
            }

            return NULL;
        }

        /**
         * @brief Moves to parent menu. Sets currently active item's parent item as active menu
         *      item.
         * @return Returns parent menu item reference or NULL if there is no parent item.
         */
        QMenuItem* back() {
            if (this->_active != NULL) {
                QMenuItem* oldActive = this->_active;
                QMenuItem* newActive = this->_active->getBack();
                if (newActive != NULL) {
                    this->_active = newActive;
                    doOnActiveItemChanged(oldActive, newActive);
                }
                return newActive;
            }

            return NULL;
        }
};

/** QMenuRenderer onItemRender event data */
struct QMenuRenderItemEvent {
    const QMenuItem* item;
    const boolean isActive;
    const int menuIndex;
    const int renderIndex;
};

/** QMenuRenderer onItemRender event */
typedef void (*QMenuOnRenderItemCallback) (QMenuRenderItemEvent);

/**
 * @brief Rendering machine for menu backend controller.
 */
class QMenuRenderer {
    private:
        QMenuOnRenderItemCallback _onRenderItem;

    protected:
        QMenu* menu;

        /**
         * @brief Calls assigned render callback.
         * @param item Menu item to be rendered.
         * @param isActive Set to true to mark rendered item as curretly selected.
         * @param menuIndex Zero based item index of whole menu.
         * @param renderIndex Zero based item index in menu viewport.
         */
        void renderItem(QMenuItem* item, boolean isActive, int menuIndex, int renderIndex) {
            if (this->_onRenderItem) {
                QMenuRenderItemEvent event = { item, isActive, menuIndex, renderIndex };
                this->_onRenderItem(event);
            }
        }

    public:
        /**
         * @brief Creates new QMenuRenderer instance.
         * @param menu Menu to redner.
         */
        QMenuRenderer(QMenu* menu) {
            this->menu = menu;
        }

        /**
         * @brief Gets render item callback called when menu item has to be rendered.
         * @return Returns onRenderItemCallbackEventCallback function reference or
         * NULL if not assigned.
         */
        QMenuOnRenderItemCallback getOnRenderItem() {
            return this->_onRenderItem;
        }

        /**
         * @brief Sets render item callback called when menu item has to be rendered.
         * @param The onRenderItemCallbackEventCallback function.
         */
        void setOnRenderItem(QMenuOnRenderItemCallback onRenderItemCallback) {
            this->_onRenderItem = onRenderItemCallback;
        }
};

/**
 * @brief List menu renderer.
 */
class QMenuListRenderer : public QMenuRenderer {
    private:
        int _viewportIndex = 0;
        int _viewportSize;

    protected:
        /**
         * @brief Calculates index of menu item in viewport.
         * @param top First item of current submenu.
         * @param active Currently active menu item.
         */
        void calcViewportIndex(QMenuItem* top, QMenuItem* active) {
            int index = 0;
            QMenuItem* item = top;
            while (item != NULL) {

                // Check active item in viewport
                if (item == active) {

                    // Active item before view port
                    if (index < this->_viewportIndex) {
                        this->_viewportIndex = index;
                        break;
                    }

                    // Active item after viewport
                    if (index >= this->_viewportIndex + this->_viewportSize) {
                        this->_viewportIndex = index - this->_viewportSize + 1;
                        break;
                    }
                }

                // Next item
                item = item->getNext();
                index++;
            }
        }

        /**
         * @brief Calls rendering for all items in viewport.
         * @param top Top item of rendered menu level.
         * @param active Currently active item in rendered menu level.
         */
        void renderItemsInViewport(QMenuItem* top, QMenuItem* active) {
            int index = 0;
            QMenuItem* item = top;
            while (item != NULL) {

                // Render item if in viewport
                if (index >= this->_viewportIndex) {
                    renderItem(item, item == active, index, index - this->_viewportIndex);
                }

                // All renderred, break
                if (index >= this->_viewportIndex + this->_viewportSize - 1) {
                    break;
                }

                // Next item
                item = item->getNext();
                index++;
            }
        }

    public:
        /**
         * @brief Creates new QMenuRenderer instance.
         * @param menu Menu to redner.
         * @param viewportSize Number of menu items mutually visible.
         */
        QMenuListRenderer(QMenu* menu, int viewportSize)
            : QMenuRenderer(menu)
        {
            this->_viewportSize = viewportSize;
        }

        /**
         * @brief Renders associated menu. Calculates items that has to be rendered to current
         * viewport and calls callback to provide user defined item
         * draw.
         */
        void render() {
            // Check menu set
            if (this->menu == NULL) {
                return;
            }

            // Check active set
            QMenuItem* active = this->menu->getActive();
            if (active == NULL) {
                return;
            }

            // Get first item in current menu
            QMenuItem* top = getTopItem(active);

            // Calc viewport position
            calcViewportIndex(top, active);

            // Render viewport
            renderItemsInViewport(top, active);
        }

        /**
         * @brief Gets top most item from current.
         * @param item Custom menu item.
         * @return Returns top most item in menu or submenu or NULL if given
         *      item is NULL.
         */
        QMenuItem* getTopItem(QMenuItem* item) {
            if (item == NULL) {
                return NULL;
            }

            QMenuItem* top = item;
            while (top->getPrev() != NULL) {
                top = top->getPrev();
            }
            return top;
        }
};

#endif
