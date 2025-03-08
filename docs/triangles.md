# Triangles in Merkle Trees
 
## Notes about Triangles
We are going to assume a 2^256 leaf merkle tree.

##  Pathing
Lets take the below example, this looks sort of like a bunc of triforces.
what is important to note though is that the impact of any change in a
leaf of an SMT can affect inner, non-edge paths unitl it reachs the tip
of an inner triange. With that said tha whole thing is made up of triangles 
of verious bases which are powers of ... 2, 

we have inner triangles with bases of width and a height of log<2>[B+1]
1,
2,
4,
8,
16,


```
I  T 0 1 2 3 4 5 6 7 8 9 A B C D E F 0 1 2 3 4 5 6 7 8 9 A B C D E F 
D0 - 0 1 0 0 0 5 0 0 0 0 0 0 0 0 0 F 0 1 0 0 0 5 0 0 0 0 0 0 0 0 0 F
D1 x  *   0   0   0   0   0   0   0   0   0   0   0   0   0   0   * 
D2 x    *       0       0       0       0       0       0       *
D3 x        *               0               0               *         
D5 x                *                               *
D6 -                               *
```


