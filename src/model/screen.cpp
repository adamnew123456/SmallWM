/** @file */
#include "screen.h"

/**
 * Finds out which screen a particular coordinate inhabits.
 */
Crt *CrtManager::screen_of_coord(Dimension x, Dimension y)
{
    for (std::map<Crt*, Box>::iterator iter = m_boxes.begin();
         iter != m_boxes.end(); iter++)
    {
        Box &value = iter->second;

        if (IN_BOUNDS(x, value.x, value.x + value.width) &&
            IN_BOUNDS(y, value.y, value.y + value.height))
            return iter->first;
    }

    return NULL;
}

/**
 * Finds the box of a particular screen.
 */
Box &CrtManager::box_of_screen(Crt *screen) 
{
    return m_boxes[screen];
}

/**
 * Finds out which screen a box is representative of.
 *
 * This *could* be made more efficient by using a second std::map, but
 * there aren't enough screens that a linear search is prohibitively
 * expensive.
 */
Crt *CrtManager::screen_of_box(const Box &box)
{
    for (std::map<Crt*, Box>::iterator iter = m_boxes.begin();
         iter != m_boxes.end(); iter++)
    {
        Box &value = iter->second;
        if (value == box)
            return iter->first;
    }

    return NULL;
}

/**
 * Rebuilds the screen graph, from a list of screen bounding boxes.
 */
void CrtManager::rebuild_graph(std::vector<Box> &screens)
{
    // The destructor for Crt traverses the entire graph
    delete m_root;
    m_boxes.clear();

    // Make sure that each box is accessible by its root coordinates
    std::map<Dimension2D, Box> origin_to_box;
    for (std::vector<Box>::iterator iter = screens.begin();
            iter != screens.end();
            iter++)
        origin_to_box[Dimension2D(iter->x, iter->y)] = *iter;

    // Start building the screen hierarchy at (0, 0) - the root screen
    m_root = new Crt();
    m_boxes[m_root] = origin_to_box[Dimension2D(0, 0)];
    
    build_node(m_root, origin_to_box);
}

/**
 * Builds up the screen graph starting from a particular screen.
 *
 * Note that this goes only down and to the right - this is because the starting
 * point is intended to be the root screen at (0, 0).
 */
void CrtManager::build_node(Crt *screen, std::map<Dimension2D, Box> &boxes)
{
    Box &my_box = m_boxes[screen];

    Dimension2D below_box(my_box.x, my_box.y + my_box.height);
    Dimension2D right_box(my_box.x + my_box.width, my_box.y);

    if (boxes.count(below_box))
    {
        Box &complete_below_box = boxes[below_box];

        Crt *below = screen_of_box(complete_below_box);
        if (!below)
        {
            below = new Crt();
            m_boxes[below] = complete_below_box;
        }

        screen->bottom = below;
        below->top = screen;

        build_node(below, boxes);
    }

    if (boxes.count(right_box))
    {
        Box &complete_right_box = boxes[right_box];

        Crt *right = screen_of_box(Box(complete_right_box));
        if (!right)
        {
            right = new Crt();
            m_boxes[right] = complete_right_box;
        }

        screen->right = right;
        right->left = screen;

        build_node(right, boxes);
    }
}
