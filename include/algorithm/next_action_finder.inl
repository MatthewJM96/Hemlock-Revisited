// template <typename ActionType>
// halgo::NextActionIteratorBase<ActionType>& halgo::NextActionMultiIterator::operator++() {
//     if ()
// }

// template <typename ActionType>
// halgo::NextActionIteratorBase<ActionType>& halgo::NextActionMultiIterator::operator++(i32) {

// }

// template <typename ActionType, typename ...ConcreteStepFinders>
// halgo::NextActionMultiIterator<ActionType> halgo::NextActionMultiFinder<ActionType, ConcreteStepFinders...>::begin() {

// }

// template <typename ActionType, typename ...ConcreteStepFinders>
// const halgo::NextActionMultiIterator<ActionType> halgo::NextActionMultiFinder<ActionType, ConcreteStepFinders...>::begin() const {
    
// }

// template <typename ActionType, typename ...ConcreteStepFinders>
// halgo::NextActionMultiIterator<ActionType> halgo::NextActionMultiFinder<ActionType, ConcreteStepFinders...>::end() {
    
// }

// template <typename ActionType, typename ...ConcreteStepFinders>
// const halgo::NextActionMultiIterator<ActionType> halgo::NextActionMultiFinder<ActionType, ConcreteStepFinders...>::end() const {
    
// }

template <typename ActionType>
halgo::NextActionFromGraphIterator<ActionType> halgo::NextActionFromGraphFinder<ActionType>::begin() {
    return m_iterator_pair.first;
}

template <typename ActionType>
const halgo::NextActionFromGraphIterator<ActionType> halgo::NextActionFromGraphFinder<ActionType>::begin() const {
    return m_iterator_pair.first;
}

template <typename ActionType>
halgo::NextActionFromGraphIterator<ActionType> halgo::NextActionFromGraphFinder<ActionType>::end() {
    return m_iterator_pair.second;
}

template <typename ActionType>
const halgo::NextActionFromGraphIterator<ActionType> halgo::NextActionFromGraphFinder<ActionType>::end() const {
    return m_iterator_pair.second;
}
