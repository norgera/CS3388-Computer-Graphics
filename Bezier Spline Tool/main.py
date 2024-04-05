import glfw
from OpenGL.GL import *
import math
from math import *
import sys


# Define a class for the nodes
class Node:
    def __init__(self, x, y):
        self.x = x
        self.y = y
        self.control_point_pair = None  # This will be a tuple of Nodes if they exist


# Global variables
control_points = []
nodes = []
selectedNode = None


# Function to adjust the control points when a node is moved
def move_control_points(node, new_x, new_y):
    if node.control_point_pair:
        num_cp = len(node.control_point_pair) # Number of control points for the node
        
        # if there is only one control point, adjust its position
        if num_cp == 1:
            cp1 = node.control_point_pair[0]
            angle = math.atan2(new_y - node.y, new_x - node.x)
            distance = math.sqrt((cp1.x - node.x) ** 2 + (cp1.y - node.y) ** 2)
            cp1.x = node.x + math.cos(angle) * distance
            cp1.y = node.y + math.sin(angle) * distance
        
        # if there are two control points, adjust both of their positions
        elif num_cp == 2:
            cp1, cp2 = node.control_point_pair # Unpack the control points
            angle = math.atan2(new_y - node.y, new_x - node.x) # Angle between the node and the new position
            distance_cp1 = math.sqrt((cp1.x - node.x) ** 2 + (cp1.y - node.y) ** 2) # Distance between the node and the first control point
            distance_cp2 = math.sqrt((cp2.x - node.x) ** 2 + (cp2.y - node.y) ** 2) # Distance between the node and the second control point

            # Adjust the position of the first control point
            cp1.x = node.x + math.cos(angle) * distance_cp1 
            cp1.y = node.y + math.sin(angle) * distance_cp1

            # Adjust the position of the second control point
            cp2.x = node.x - math.cos(angle) * distance_cp2
            cp2.y = node.y - math.sin(angle) * distance_cp2


# Callback function for mouse button events
def mouse_callback(window, button, action, mods):

    global selectedNode, nodes, control_points

    if button != glfw.MOUSE_BUTTON_LEFT:
        return

    # Get the position of the mouse
    mouseX, mouseY = glfw.get_cursor_pos(window)
    mouseY = window_height - mouseY  # Convert to OpenGL coordinates


    # If the left mouse button was pressed
    if action == glfw.PRESS:

        # Check if control point was clicked
        for cp in control_points:
            if math.hypot(cp.x - mouseX, cp.y - mouseY) < 10.0:
                selectedNode = ('control_point', cp)
                return
            
        # Check if a node was clicked
        for node in nodes:
            if math.hypot(node.x - mouseX, node.y - mouseY) < 10.0:
                selectedNode = ('node', node)
                return

        # If no node or control point was clicked, add a new node
        new_node = Node(mouseX, mouseY)
        nodes.append(new_node)

        # If there is only one node, add a control point 50 pixels above it
        if len(nodes) == 1:
            cp = Node(new_node.x, new_node.y + 50)
            new_node.control_point_pair = (cp,)
            control_points.append(cp)

        # If there are two nodes, add a control point 50 pixels above the second node
        else:
            cp = Node(new_node.x, new_node.y + 50)
            new_node.control_point_pair = (cp,)
            control_points.append(cp)

            # Determine the closer endpoint to make it an intermediate node
            start_node = nodes[0]
            end_node = nodes[-2]  # The last node before adding the new one

            # Determine which node is closer to the mouse click
            start_dist = math.hypot(start_node.x - mouseX, start_node.y - mouseY)
            end_dist = math.hypot(end_node.x - mouseX, end_node.y - mouseY)

            intermediate_node = start_node if start_dist < end_dist else end_node # The closer node

            # If the intermediate node is not the first node, add a control point 50 pixels below it
            if intermediate_node != nodes[0]: 
                if not intermediate_node.control_point_pair or len(intermediate_node.control_point_pair) == 1:
                    existing_cp = intermediate_node.control_point_pair[0]
                    new_cp = Node(intermediate_node.x, intermediate_node.y - 50)

                    # Add the new control point to the control_points list
                    intermediate_node.control_point_pair = (existing_cp, new_cp)
                    control_points.append(new_cp)

    elif action == glfw.RELEASE:
        selectedNode = None



# Callback function for cursor position
def cursor_callback(window, xpos, ypos):
    global nodes, control_points
    ypos = window_height - ypos  # Convert to OpenGL coordinates
    if selectedNode:
        selection_type, selected_obj = selectedNode
        

        # If a node is selected, adjust its position and control points
        if selection_type == 'node':
            dx = xpos - selected_obj.x
            dy = ypos - selected_obj.y
            selected_obj.x = xpos
            selected_obj.y = ypos

            # Adjust the control points for the node
            if selected_obj.control_point_pair:
                for cp in selected_obj.control_point_pair:
                    cp.x += dx
                    cp.y += dy

        # Adjust the control points for the other nodes
        elif selection_type == 'control_point':
            selected_obj.x = xpos
            selected_obj.y = ypos
            # Adjust the sibling control point for interior nodes
            for node in nodes:
                if node.control_point_pair and selected_obj in node.control_point_pair:

                    # Find the sibling control point
                    if len(node.control_point_pair) == 2:
                        cp1, cp2 = node.control_point_pair # Unpack the control points
                        if selected_obj == cp1:
                            sibling_cp = cp2
                        else:
                            sibling_cp = cp1

                        # Calculate new position for sibling control point
                        dx = node.x - selected_obj.x
                        dy = node.y - selected_obj.y
                        sibling_cp.x = node.x + dx
                        sibling_cp.y = node.y + dy
                    break


# Function to draw a circle
def draw_circle(centerX, centerY, radius, segments):
    glBegin(GL_TRIANGLE_FAN)
    glVertex2f(centerX, centerY)  # Center of circle

    for i in range(segments + 1):
        angle = 2.0 * pi * float(i) / float(segments) 
        x = radius * cos(angle) 
        y = radius * sin(angle)
        glVertex2f(x + centerX, y + centerY) 
    glEnd()


# Function to draw the nodes and control points
def draw_nodes_and_control_points(nodes, control_points, window_width, window_height):
    glPointSize(20.0)
    glColor3f(0.0, 0.0, 1.0)
    glBegin(GL_POINTS)

    # Draw nodes as blue points
    for node in nodes:
        nx = node.x / (window_width / 2.0) - 1.0 # Normalize the x-coordinate
        ny = node.y / (window_height / 2.0) - 1.0 # Normalize the y-coordinate
        glVertex2f(nx, ny) # Draw the node
    glEnd()

    # Draw control points as circles
    glColor3f(0.0, 0.0, 0.0)
    for cp in control_points:
        nx = cp.x / (window_width / 2.0) - 1.0
        ny = cp.y / (window_height / 2.0) - 1.0
        draw_circle(nx, ny, 0.02, 32)
    glDisable(GL_BLEND)



# Callback function for clearing the screen
def clear_screen(window, key, scancode, action, mods):
    global nodes, control_points, selectedNode
    if key == glfw.KEY_E and action == glfw.PRESS: # Clear the screen when the 'E' key is pressed
        nodes.clear()
        control_points.clear()
        selectedNode = None 


# Function to draw a Bezier curve
def draw_bezier_curve(x0, y0, x1, y1, cx1, cy1, cx2, cy2):
    glBegin(GL_LINE_STRIP)
    for i in range(201):
        t = i / 200.0
        xt = (1-t)**3 * x0 + 3 * t * (1-t)**2 * cx1 + 3 * t**2 * (1-t) * cx2 + t**3 * x1
        yt = (1-t)**3 * y0 + 3 * t * (1-t)**2 * cy1 + 3 * t**2 * (1-t) * cy2 + t**3 * y1
        glVertex2f(xt, yt)
    glEnd()


# Function to draw the lines connecting the control points to the nodes
def draw_control_point_lines():
    glColor3f(0.0, 0.0, 0.0)
    for node in nodes:
        if node.control_point_pair:
            for cp in node.control_point_pair:

                # Normalize the coordinates
                x0 = node.x / (window_width / 2) - 1.0
                y0 = node.y / (window_height / 2) - 1.0

                # Normalize the coordinates of the control point
                x1 = cp.x / (window_width / 2) - 1.0
                y1 = cp.y / (window_height / 2) - 1.0

                # Draw a dashed line between the node and its control point
                draw_dotted_line(x0, y0, x1, y1)


# Function to draw a dashed line
def draw_dotted_line(x0, y0, x1, y1):
    glEnable(GL_LINE_STIPPLE)
    glLineStipple(1, 0x00FF)
    glColor3f(0.0, 1.0, 1.0)
    glBegin(GL_LINES)
    glVertex2f(x0, y0)
    glVertex2f(x1, y1)
    glEnd()
    glDisable(GL_LINE_STIPPLE)
        
# Function to draw the Bezier curves
def draw_splines():
    if len(nodes) < 2: # Need at least two nodes to draw a curve
        return

    glEnable(GL_LINE_SMOOTH)
    glEnable(GL_BLEND)
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)
    glLineWidth(4)
    glColor3f(0.0, 0.0, 0.0)


    # Draw the Bezier curves for each segment
    for i in range(1, len(nodes)):

        # Normalize the coordinates
        x0 = nodes[i-1].x / (window_width / 2.0) - 1.0 
        y0 = nodes[i-1].y / (window_height / 2.0) - 1.0 
        x3 = nodes[i].x / (window_width / 2.0) - 1.0 
        y3 = nodes[i].y / (window_height / 2.0) - 1.0

       
        # Set the control points to the nodes
        cx1, cy1 = x0, y0
        cx2, cy2 = x3, y3

        # Use the control points of the nodes
        if nodes[i-1].control_point_pair:
            cp1 = nodes[i-1].control_point_pair[0]  # First control point for the starting node
            cx1 = cp1.x / (window_width / 2.0) - 1.0
            cy1 = cp1.y / (window_height / 2.0) - 1.0
        if nodes[i-1].control_point_pair and len(nodes[i-1].control_point_pair) > 1:
            cp1 = nodes[i-1].control_point_pair[1]  # Second control point, if it exists
            cx1 = cp1.x / (window_width / 2.0) - 1.0
            cy1 = cp1.y / (window_height / 2.0) - 1.0
            
        if nodes[i].control_point_pair:
            cp2 = nodes[i].control_point_pair[0]
            cx2 = cp2.x / (window_width / 2.0) - 1.0
            cy2 = cp2.y / (window_height / 2.0) - 1.0

        draw_bezier_curve(x0, y0, x3, y3, cx1, cy1, cx2, cy2)

    glDisable(GL_BLEND)
    glDisable(GL_LINE_SMOOTH)






def main():
    if len(sys.argv) < 3:
        print("Usage: python <scriptname>.py <width> <height>")
        return

    global window_width, window_height
    try:
        window_width = int(sys.argv[1])
        window_height = int(sys.argv[2])
    except ValueError:
        print("Please provide integer values for width and height.")
        return

    if not glfw.init():
        return

    window = glfw.create_window(window_width, window_height, "Bezier Spline Tool", None, None)
    if not window:
        glfw.terminate()
        return

    glfw.make_context_current(window)
    
    # Register callbacks
    glfw.set_mouse_button_callback(window, mouse_callback)
    glfw.set_cursor_pos_callback(window, cursor_callback)
    glfw.set_key_callback(window, clear_screen)

    glClearColor(1.0, 1.0, 1.0, 1.0) # Set the background color to white
    while not glfw.window_should_close(window):
        glfw.poll_events()
        glClear(GL_COLOR_BUFFER_BIT)

        # Draw the nodes and control points
        draw_nodes_and_control_points(nodes, control_points, window_width, window_height)
        draw_splines()
        draw_control_point_lines()
        glfw.swap_buffers(window)

    glfw.terminate()

if __name__ == "__main__":
    main()


