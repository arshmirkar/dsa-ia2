#include <stdio.h>
#include <stdlib.h>


// --- CORE DATA STRUCTURES ---
typedef struct { float x, y; } Point;
typedef struct { Point center; float halfDim; } Boundary;
typedef struct { int plotID; Point pos; } FarmPlot;


// Priority Queue for Irrigation Advisories
typedef struct {
    int plotID;
    int urgency; // Higher number = higher priority
} Advisory;


typedef struct {
    Advisory queue[100];
    int size;
} PriorityQueue;


// --- PRIORITY QUEUE OPERATIONS ---
void add_advisory(PriorityQueue* pq, int plotID, int urgency) {
    pq->queue[pq->size].plotID = plotID;
    pq->queue[pq->size].urgency = urgency;
    pq->size++;
}


Advisory get_next_advisory(PriorityQueue* pq) {
    int maxIdx = 0;
    // Simple linear search to find highest priority for demonstration
    for(int i = 1; i < pq->size; i++) {
        if(pq->queue[i].urgency > pq->queue[maxIdx].urgency) {
            maxIdx = i;
        }
    }
    Advisory highest = pq->queue[maxIdx];
    pq->queue[maxIdx] = pq->queue[pq->size - 1]; // Replace with last element
    pq->size--;
    return highest;
}
// --- QUAD-TREE STRUCTURE ---
typedef struct QuadNode {
    Boundary boundary;
    FarmPlot plot;
    int hasPlot; // 0 if empty, 1 if holds a plot
    struct QuadNode *NW, *NE, *SW, *SE;
} QuadNode;


QuadNode* create_node(Boundary b) {
    QuadNode* n = (QuadNode*)malloc(sizeof(QuadNode));
    n->boundary = b; n->hasPlot = 0;
    n->NW = n->NE = n->SW = n->SE = NULL;
    return n;
}


int in_boundary(Point p, Boundary b) {
    return (p.x >= b.center.x - b.halfDim && p.x <= b.center.x + b.halfDim &&
            p.y >= b.center.y - b.halfDim && p.y <= b.center.y + b.halfDim);
}


// --- QUAD-TREE INSERTION ---
void insert_plot(QuadNode* node, FarmPlot p) {
    if (!in_boundary(p.pos, node->boundary)) return; // Plot outside this node


    // If node is empty and has no children, put plot here
    if (node->hasPlot == 0 && node->NW == NULL) {
        node->plot = p;
        node->hasPlot = 1;
        return;
    }


    // Subdivide if not already divided
    if (node->NW == NULL) {
        float hd = node->boundary.halfDim / 2.0;
        Point c = node->boundary.center;
        node->NW = create_node((Boundary){{c.x - hd, c.y + hd}, hd});
        node->NE = create_node((Boundary){{c.x + hd, c.y + hd}, hd});
        node->SW = create_node((Boundary){{c.x - hd, c.y - hd}, hd});
        node->SE = create_node((Boundary){{c.x + hd, c.y - hd}, hd});
       
        // Move existing plot to a child
        node->hasPlot = 0;
        insert_plot(node->NW, node->plot); insert_plot(node->NE, node->plot);
        insert_plot(node->SW, node->plot); insert_plot(node->SE, node->plot);
    }


    // Insert the new plot into the correct child
    insert_plot(node->NW, p); insert_plot(node->NE, p);
    insert_plot(node->SW, p); insert_plot(node->SE, p);
}
// --- QUAD-TREE SEARCH ---
int intersect(Boundary b1, Boundary b2) {
    return !(b2.center.x - b2.halfDim > b1.center.x + b1.halfDim ||
             b2.center.x + b2.halfDim < b1.center.x - b1.halfDim ||
             b2.center.y - b2.halfDim > b1.center.y + b1.halfDim ||
             b2.center.y + b2.halfDim < b1.center.y - b1.halfDim);
}


void search_region(QuadNode* node, Boundary range) {
    if (node == NULL || !intersect(node->boundary, range)) return;


    // If there is a plot here, check if it falls inside the search range
    if (node->hasPlot && in_boundary(node->plot.pos, range)) {
        printf("Found Farm Plot ID: %d at coords (%.1f, %.1f)\n",
                node->plot.plotID, node->plot.pos.x, node->plot.pos.y);
    }
        
    // Continue searching children
    search_region(node->NW, range); search_region(node->NE, range);
    search_region(node->SW, range); search_region(node->SE, range);
}


// --- MAIN EXECUTION ---
int main() {
    // 1. Initialize map (Center at 50,50 with half-dimension of 50 -> covers 0 to 100)
    Boundary map = {{50.0, 50.0}, 50.0};
    QuadNode* root = create_node(map);
    PriorityQueue advisoryQueue = {.size = 0};


    // 2. Insert geofenced plots (Person 2's part)
    insert_plot(root, (FarmPlot){1001, {20.0, 20.0}});
    insert_plot(root, (FarmPlot){1002, {80.0, 80.0}});
    insert_plot(root, (FarmPlot){1003, {25.0, 30.0}});


    // 3. Search for plots in a specific region (Person 3's part)
    printf("--- Spatial Search ---\n");
    printf("Searching SW region for plots...\n");
    Boundary searchArea = {{25.0, 25.0}, 25.0}; // Covers roughly 0 to 50
    search_region(root, searchArea);


    // 4. Manage Advisories (Person 1's part)
    printf("\n--- Irrigation Advisories ---\n");
    add_advisory(&advisoryQueue, 1001, 5); // Plot 1001, moderate stress
    add_advisory(&advisoryQueue, 1003, 9); // Plot 1003, high stress
   
    Advisory urgent = get_next_advisory(&advisoryQueue);
    printf("Dispatching highest priority advisory to Plot ID: %d (Urgency Level: %d)\n",
            urgent.plotID, urgent.urgency);


    return 0;
}
