# Bitwise Path Isolation Algorithm [WIP]

## Overview
An algorithm for determining the intersection of leaves in a Sparse Merkle Tree. The algorithm uses binary representation with a bitmask to efficiently determine path intersections.

## Algorithm Description

The algorithm works by masking off bits from LSB (Least Significant Bit) to MSB (Most Significant Bit) and looking for equalities. This approach allows us to determine a region of isolation where anything under a specific depth value can safely be calculated as if it were a single element in a tree, eliminating the need to worry about collisions.

Add 1 for even depth values greater than 3 and also form a triange tip...

## Example 1. (2 Leaves)

Consider two leaves at positions 1 and 5 with a depth of 3:

```
leaf 1: 00000001 [0,1]
leaf 5: 00000101 [4,5]
```

We mask off the bits one by one:

### Step 1: Depth(1)
```
0000000_[1]
0000010_[1]
```
Result: Not equal, so there is no interference at this level.

### Step 2: Depth(2)
```
000000_[01]
000001_[01]
```
Result: Not equal, so there is no interference at this level.

### Step 3: Depth(3)
```
00000_[001]
00000_[101]
```
Result: Equal prefix, so there is interference at this level.

## Visual Representation (Isolated Tree)

```
D0 - 0 1 0 0 0 5 0 0 | n(256)
D1 -  *   0   *   0
D2 -    *       *
D3 -        *
```

## Example 2. (3 Leaves) !!! Broken !!!

Consider two leaves at positions 1 and 5 with a depth of 3:

```
leaf 1:  00000001 [0,1]
leaf 5:  00000101 [4,5]
leaf 15: 00000001 [14,15]
```

We mask off the bits one by one:

### Step 1: Depth(1)
```
0000000_[1]
0000010_[1]
0000111_[1]
```
Result: Not equal, so there is no interference at this level.

### Step 2: Depth(2)
```
000000_[01]
000001_[01]
000011_[11]
```
Result: Not equal, so there is no interference at this level.

### Step 3: Depth(3)
```
00000_[001]
00000_[101]
00001_[111]
```
Result: Partial Equal prefix, so there is Some interference at this level between members.

### Step 4: Depth(4)
```
0000_[0001]
0000_[0101]
0000_[1111]
```
Result: Equal prefix, so anything above this value can be just matched with null hashes

## Visual Representation (Isolated Tree) [15 = F]

```
I  - 0 1 2 3 4 5 6 7 8 9 A B C D E F
D0 - 0 1 0 0 0 5 0 0 0 0 0 0 0 0 0 F| ... n(256)
D1 -  *   0   *   0   0   0   0   *
D2 -    *       *       0       *
D3 -        *               *
D4 -            *       *
D5 -                *
```

## Example 1. (2 Leaves)

Consider two leaves at positions 1 and 5 with a depth of 3:

```
leaf 1: 00000001 [0,1]
leaf 5: 00000010 [2,3]
```

We mask off the bits one by one:

### Step 1: Depth(1)
```
0000000_[1]
0000001_[0]
```
Result: Not equal, so there is no interference at this level.

### Step 2: Depth(2)
```
000000_[01]
000000_[01]
```
Result: Equal prefix, so there is interference at this level.

## Visual Representation (Isolated Tree)

```
D0 - 0 1 2 0 0 0 0 0 | n(256)
D1 -  *   *   0   0
D2 -    *       0
```


##
## Visual Representation (Isolated Tree) [6 items w/ > n^2-2]

1,5,15,17,20,31

```
I  - 0 1 2 3 4 5 6 7 8 9 A B C D E F 0 1 2 3 4 5 6 7 8 9 A B C D E F 
D0 - 0 1 0 0 0 5 0 0 0 0 0 0 0 0 0 F 0 1 0 0 0 5 0 0 0 0 0 0 0 0 0 F
D1 -  *   0   *   0   0   0   0   *   *   0   *   0   0   0   0   * 
D2 -    *       *       0       *       *       *       0       *
D3 -        *               *               *               *         
D4 -            *       *                       *       *
D5 -                *                               *
D6 -                               *
```

##
## Visual Representation (Isolated Tree) [2 items]

1, 31

```
I  - 0 1 2 3 4 5 6 7 8 9 A B C D E F 0 1 2 3 4 5 6 7 8 9 A B C D E F 
D0 - 0 1 0 0 0 5 0 0 0 0 0 0 0 0 0 F 0 1 0 0 0 5 0 0 0 0 0 0 0 0 0 F
D1 x  *   0   0   0   0   0   0   0   0   0   0   0   0   0   0   * 
D2 x    *       0       0       0       0       0       0       *
D3 x        *               0               0               *         
D4 -            *       0                       0       *
D5 x                *                               *
D6 -                               *
```

## Implementation Notes

This algorithm is particularly useful in Sparse Merkle Trees where we need to efficiently determine which paths intersect. By identifying the depth at which paths begin to diverge, we can optimize operations like proof generation and validation.
