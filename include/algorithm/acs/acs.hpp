#include "graph/acs.hpp"

// TODO(Matthew): Support version with runtime paramterisation?
// TODO(Matthew): More intelligent use of mean filtering versus dynamic exploitation
//                (first is a very local increase of exploration where second is
//                global).
// TODO(Matthew): Don't necessarily want to provide a graph map - scenarios exist where
//                no sharing can or will occur and thus it may be faster to deal simply
//                with VertexData.
// TODO(Matthew): Can we drop pheromone proportionate to some metric (naively degree of
//                distance reduction on path) so long as no ant has yet actually reached
//                the target?
// TODO(Matthew): Allow a variation where the destination is a comparator.
//                  Note that here we will need to allow providing an optional metric
//                  for determining distance, and where not given the algorithm will
//                  have to disable some components.
