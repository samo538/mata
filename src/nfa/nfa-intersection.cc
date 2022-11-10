/* nfa-intersection.cc -- Intersection of NFAs
 *
 * This file is a part of libmata.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

// MATA headers
#include <mata/nfa.hh>

using namespace Mata::Nfa;

namespace {

void union_to_left(StateSet &receivingSet, const StateSet &addedSet) {
    receivingSet.insert(addedSet);
}

/**
 * Add transition to the product.
 * @param[out] product Created product automaton.
 * @param[out] product_map Created product map.
 * @param[in] pair_to_process Currently processed pair of original states.
 * @param[in] intersection_transition State transitions to add to the product.
 */
void add_product_transition(Nfa& product, std::unordered_map<std::pair<State,State>, State>& product_map,
                            const std::pair<State,State>& pair_to_process,
                            Move& intersection_transition) {
    if (intersection_transition.states_to.empty()) { return; }

    auto& intersect_state_transitions{ product.transition_relation[product_map[pair_to_process]] };
    auto symbol_transitions_iter{ intersect_state_transitions.find(intersection_transition) };
    if (symbol_transitions_iter == intersect_state_transitions.end()) {
        intersect_state_transitions.push_back(intersection_transition);
    } else {
        // Product already has some target states for the given symbol from the current product state.
        union_to_left(symbol_transitions_iter->states_to, intersection_transition.states_to);
    }
}

/**
 * Create product state and its transitions.
 * @param[out] product Created product automaton.
 * @param[out] product_map Created product map.
 * @param[out] pairs_to_process Set of product states to process
 * @param[in] lhs_state_to Target state in NFA @c lhs.
 * @param[in] rhs_state_to Target state in NFA @c rhs.
 * @param[out] intersect_transitions Transitions of the product state.
 */
void create_product_state_and_trans(
            Nfa& product,
            std::unordered_map<std::pair<State,State>, State>& product_map,
            const Nfa& lhs,
            const Nfa& rhs,
            std::unordered_set<std::pair<State,State>>& pairs_to_process,
            const State lhs_state_to,
            const State rhs_state_to,
            Move& intersect_transitions
) {
    const std::pair<State,State> intersect_state_pair_to(lhs_state_to, rhs_state_to);
    State intersect_state_to;
    if (product_map.find(intersect_state_pair_to) == product_map.end()) {
        intersect_state_to = product.add_state();
        product_map[intersect_state_pair_to] = intersect_state_to;
        pairs_to_process.insert(intersect_state_pair_to);

        if (lhs.has_final(lhs_state_to) && rhs.has_final(rhs_state_to)) {
            product.add_final(intersect_state_to);
        }
    } else {
        intersect_state_to = product_map[intersect_state_pair_to];
    }
    intersect_transitions.states_to.insert(intersect_state_to);
}

} // Anonymous namespace.

namespace Mata {
namespace Nfa {

Nfa intersection(const Nfa& lhs, const Nfa& rhs, bool preserve_epsilon,
                 std::unordered_map<std::pair<State,State>, State> *prod_map) {
    Nfa product{}; // Product of the intersection.
    // Product map for the generated intersection mapping original state pairs to new product states.
    std::unordered_map<std::pair<State,State>, State> product_map{};
    std::pair<State,State> pair_to_process{}; // State pair of original states currently being processed.
    std::unordered_set<std::pair<State,State>> pairs_to_process{}; // Set of state pairs of original states to process.

    // Initialize pairs to process with initial state pairs.
    for (const State lhs_initial_state : lhs.initial_states) {
        for (const State rhs_initial_state : rhs.initial_states) {
            // Update product with initial state pairs.
            const std::pair<State,State> this_and_other_initial_state_pair(lhs_initial_state, rhs_initial_state);
            const State new_intersection_state = product.add_state();

            product_map[this_and_other_initial_state_pair] = new_intersection_state;
            pairs_to_process.insert(this_and_other_initial_state_pair);

            product.initial_states.push_back(new_intersection_state);
            if (lhs.has_final(lhs_initial_state) && rhs.has_final(rhs_initial_state)) {
                product.final_states.push_back(new_intersection_state);
            }
        }
    }

    while (!pairs_to_process.empty()) {
        pair_to_process = *pairs_to_process.cbegin();
        pairs_to_process.erase(pair_to_process);
        // Compute classic product for current state pair.
        Mata::Util::SynchronizedUniverzalIterator<Move> sui(2);
        sui.push_back(lhs.transition_relation[pair_to_process.first]);
        sui.push_back(rhs.transition_relation[pair_to_process.second]);

        while (sui.advance()) {
            std::vector<Moves::const_iterator> moves = sui.get_current();
            assert(moves.size() == 2); // One move per state in the pair.

            // Compute product for state transitions with same symbols.
            // Find all transitions that have the same symbol for first and the second state in the pair_to_process.
            // Create transition from the pair_to_process to all pairs between states to which first transition goes
            //  and states to which second one goes.
            Move intersection_transition{moves[0]->symbol};
            for (const State this_state_to: moves[0]->states_to)
            {
                for (const State other_state_to: moves[1]->states_to)
                {
                    create_product_state_and_trans(
                            product, product_map, lhs, rhs, pairs_to_process,
                            this_state_to, other_state_to, intersection_transition
                    );
                }
            }
            add_product_transition(product, product_map, pair_to_process, intersection_transition);
        }

        if (preserve_epsilon) {
            // Add transitions of the current state pair for an epsilon preserving product.

            // Check for lhs epsilon transitions.
            const auto& lhs_state_symbol_transitions{ lhs.transition_relation[pair_to_process.first] };
            if (!lhs_state_symbol_transitions.empty()) {
                const auto& lhs_state_last_transitions{ lhs_state_symbol_transitions.back() };
                if (lhs_state_last_transitions.symbol == EPSILON) {
                    // Compute product for state transitions with lhs state epsilon transition.
                    // Create transition from the pair_to_process to all pairs between states to which first transition
                    //  goes and states to which second one goes.
                    Move intersection_transition{EPSILON};
                    for (const State lhs_state_to: lhs_state_last_transitions.states_to) {
                        create_product_state_and_trans(product, product_map, lhs, rhs, pairs_to_process,
                                                       lhs_state_to, pair_to_process.second,
                                                       intersection_transition);
                    }
                    add_product_transition(product, product_map, pair_to_process, intersection_transition);
                }
            }

            // Check for rhs epsilon transitions in case only rhs has any transitions and add them.
            const auto& rhs_state_symbol_transitions{ rhs.transition_relation[pair_to_process.second]};
            if (!rhs_state_symbol_transitions.empty()) {
                const auto& rhs_state_last_transitions{rhs_state_symbol_transitions.back()};
                if (rhs_state_last_transitions.symbol == EPSILON) {
                    // Compute product for state transitions with rhs state epsilon transition.
                    // Create transition from the pair_to_process to all pairs between states to which first transition
                    //  goes and states to which second one goes.
                    Move intersection_transition{EPSILON};
                    for (const State rhs_state_to: rhs_state_last_transitions.states_to) {
                        create_product_state_and_trans(product, product_map, lhs, rhs, pairs_to_process,
                                                       pair_to_process.first, rhs_state_to,
                                                       intersection_transition);
                    }
                    add_product_transition(product, product_map, pair_to_process, intersection_transition);
                }
            }
        }
    }

    if (prod_map != nullptr) { *prod_map = product_map; }
    return product;
} // intersection().

} // namespace Nfa.
} // namespace Mata.
