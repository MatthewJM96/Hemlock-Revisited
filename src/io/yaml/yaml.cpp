#include "stdafx.h"

#include "io/yaml/yaml.h"

YAML::Node hio::yaml::merge(const YAML::Node left, const YAML::Node right) {
    // If the right-hand yaml document is not a map, then it wins
    // the fight so long as it is not null.
    if (!right.IsMap()) {
        return right.IsNull() ? left : right;
    }

    // If the left-hand yaml document is not a map, and we now
    // know the right-hand is a map, the right-hand document wins.
    if (!left.IsMap()) {
        return right;
    }

    // If the right-hand yaml document is empty, the merge result
    // would be identical to the left-hand.
    if (!right.size()) {
        return left;
    }

    // We must now do a more involved merge and we will store the
    // result here.
    YAML::Node result = YAML::Node(YAML::NodeType::Map);

    // Iterate left-hand document's top-most layer and determine if
    // recursive merging is needed for each element.
    for (auto left_child : left) {
        // NOTE(Matthew): we have no way to assess the equivalence
        //                of nodes that are not scalar, and as keys
        //                are represented as nodes, we therefore
        //                only consider merging cases with scalar keys.
        if (left_child.first.IsScalar()) {
            auto& key = left_child.first.Scalar();

            // Only have to do a recursive merge if the right-hand
            // document also contains a same-key child.
            auto right_child = right[key];
            if (right_child) {
                result[key] = merge(left_child.second, right_child);
                continue;
            }
        }
        // If we get here then either the left-hand child's key was
        // non-scalar or the right-hand document had no same key
        // child. We just put the left-hand child in.
        result[left_child.first] = left_child.second;
    }

    // Now we place every child in the right-hand document into
    // the result document with no concern of needing to merge.
    for (auto right_child : right) {
        // Note the const-cast is needed as yaml-cpp offers no contains method and
        // non-const operator[] creates an entry so would always fail this test.
        if (!right_child.first.IsScalar() || !make_const(result)[right_child.first.Scalar()]) {
            result[right_child.first] = right_child.second;
        }
    }

    return result;
}
