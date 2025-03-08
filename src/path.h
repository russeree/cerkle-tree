/**
 * Path Finding Algorithm: Determines the intersection of leaves
 * We should be able to use the binary representation with a bitmask to determine path intersections
 * 
 * This would be done in the way of masking off bits from LSB to MSB and looking for equalities.
 * The absolute reason for this is you can determine a reigon of isolation where anything under the
 * depth value can safely be calculated as if it were a single element in a tree, Aka. no need to worry
 * about collisions.
 * 
 *  Ex. 1 (depth 3) 
 *    leaf 1: 00000001 [0,1]
 *    leaf 5: 00000101 [4,5]
 * 
 *  So now lets mask off the bits one by 1
 *    step Depth(1) != so there is no interference at this level 
 *    0000000_[1]
 *    0000010_[1]
 * 
 *    step Depth(2) != so there is no interference at this level 
 *    000000_[01]
 *    000001_[01]
 * 
 *    step Depth(3) == so there is no interference at this level 
 *    00000_[001]
 *    00000_[101]
 * 
 *    Visual (Isolated Tree)
 *    D0 - 0 1 0 0 0 5 0 0 | n(256)
 *    D1 -  *   0   *   0
 *    D2 -    *       *
 *    D3 -        *
 */      
