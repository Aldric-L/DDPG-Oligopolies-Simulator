//
//  MatrixInterface.hpp
//  AKML Project
//
//  Created by Aldric Labarthe on 17/12/2023.
//

#ifndef AKML_MatrixInterface_hpp
#define AKML_MatrixInterface_hpp

namespace akml {

// Matrixable concept accepts pointers only for compatibility reasons, it will in future updates be an alias of Arithmetic. Matrices should not contain pointers. It's a bad idea.
template <typename element_type>
concept Matrixable = std::is_arithmetic<element_type>::value || std::is_pointer<element_type>::value;

template <typename element_type>
concept Arithmetic = std::is_arithmetic<element_type>::value;

class AbstractMatrix {};

template <akml::Matrixable element_type>
class MatrixInterface : public AbstractMatrix{
protected:
    element_type* m_data = nullptr;
    element_type* m_data_end = nullptr;
    std::size_t rows, columns;
    
public:
    static inline std::function<element_type(element_type, std::size_t, std::size_t)> NO_ACTION_TRANSFORM = [](element_type x, std::size_t row, std::size_t column) {return x;};
    
    static inline std::function<element_type(element_type, std::size_t, std::size_t)> IDENTITY_TRANSFORM = [](element_type x, std::size_t row, std::size_t column) { return (column == row) ? static_cast<element_type>(1) : static_cast<element_type>(0);};
    
    static inline std::function<element_type(element_type, std::size_t, std::size_t)> RANDOM_TRANSFORM = [](element_type x, std::size_t row, std::size_t column) { std::random_device rd;  std::mt19937 gen(rd()); std::normal_distribution<double> distribution(0,0.3); return distribution(gen); };
    
    MatrixInterface(const std::size_t rows, const std::size_t columns) : rows(rows), columns(columns) {};
    
    virtual void create() = 0;
    virtual void createInternStorage() = 0;
    virtual void deleteInternStorage() = 0;
    
    inline std::size_t getNColumns() const { return this->columns; }
    inline std::size_t getNRows() const { return this->rows; }
    
    inline bool isInitialized() const { return !(this->m_data==nullptr); }
    // FORBIDDEN NOW
    //inline element_type*& getStorage() { return this->m_data; }
    //inline element_type*& getStorageEnd() { return this->m_data_end; }
    inline element_type* getStorage() const { return this->m_data; }
    inline element_type* getStorageEnd() const { return this->m_data_end; }
    inline std::size_t getStorageLen() const { return (this->rows) * (this->columns); }
    inline element_type* getInternElement(const std::size_t pos) const {
        if (pos >= (this->rows) * (this->columns))
            throw std::invalid_argument("Attempt to access to an out of reach element of a matrix");
        
        return (this->m_data + pos);
    }
    
    inline void setMDataPointer(element_type* array_start, std::size_t sizeof_container=0){
        this->m_data = array_start;
        this->m_data_end = this->m_data + ((sizeof_container == 0 && array_start != nullptr) ? (this->rows)*(this->columns) : sizeof_container);
    }
    
    inline element_type& operator[](const std::array<std::size_t, 2> row_and_col) const {
        if (!this->isInitialized())
            throw std::invalid_argument("Matrix provided is not initialized.");
        if (this->rows <= row_and_col[0] || this->columns <= row_and_col[1])
            throw std::invalid_argument("Attempt to access to an out of reach element of a matrix");
        return *(this->m_data + row_and_col[0]*(this->columns)+row_and_col[1]);
    }
    
    inline element_type read(const size_t row, const size_t column) const {
        if (!this->isInitialized())
            throw std::invalid_argument("Matrix provided is not initialized.");
        if (this->getNRows() < row || this->getNColumns() < column)
            throw std::invalid_argument("Attempt to access to an out of reach element of a matrix");
        return *(this->m_data + (row-1)*(this->columns)+(column-1));
    }
    
    inline element_type& operator[](const std::array<std::size_t, 2> row_and_col) {
        return *(this->getInternElement(row_and_col[0]*(this->columns)+row_and_col[1]));
    }
    
    inline element_type read(const std::array<std::size_t, 2> row_and_col) const { return operator[]({row_and_col[0]-1, row_and_col[1]-1}); }
    
    inline element_type& operator()(const size_t row, const size_t column) { return operator[]({row-1, column-1}); }
    
    inline bool is_squared(){ return (this->getNColumns() == this->getNRows()); }
    
    inline void transform(const std::function<element_type(element_type, std::size_t, std::size_t)> transfunc){
        for (std::size_t i(0); i < (this->rows); i++){
            for (std::size_t j(0); j < (this->columns); j++){
                operator[]({i, j}) = transfunc(operator[]({i, j}), i, j);
            }
        }
    }
    
    inline void transform(const std::function<element_type(element_type)> transfunc){
        for (std::size_t i(0); i < (this->rows)*(this->columns); i++){
            *(getInternElement(i)) = transfunc(*getInternElement(i));
        }
    }
    
    inline static void cout(const MatrixInterface<element_type>& matrix){
        //std::cout << "Printing Matrix <" << matrix.getNRows() << " " << matrix.getNColumns() << ">" << std::endl;
        std::cout << "[";
        for (std::size_t i(0); i < matrix.getNRows(); i++){
            if (i != 0)
                std::cout << " ";
            std::cout << "[ ";
            for (std::size_t j(0); j<matrix.getNColumns(); j++){
                std::cout << matrix.read(i+1,j+1) << " ";
                if (j != matrix.getNColumns()-1)
                    std::cout << ", ";
            }
            if (i != matrix.getNRows()-1)
                std::cout << "],\n";
            else
                std::cout << "]";
        }
        std::cout << "]" << std::endl;
    }
    
    inline bool operator==(const MatrixInterface<element_type>& rhs) const {
        if (!rhs.isInitialized())
            throw std::invalid_argument("Matrix provided is not initialized.");
        
        element_type* tar_other(rhs.getStorage());
        for (element_type* tar(getStorage()); tar < getStorageEnd(); tar++){
            if (*tar != *tar_other)
                return false;
            tar_other++;
        }
        
        return true;
    }
};

class FSMatrixInterface {};

template <class T>
concept MatrixConcept = std::is_base_of<AbstractMatrix, T>::value;

template <class T>
concept FixedSizeMatrixConcept = std::is_base_of<FSMatrixInterface, T>::value;

template <class T, typename MATRIX_INNER_TYPE>
concept MatrixInterfaceConcept = std::is_base_of<MatrixInterface<MATRIX_INNER_TYPE>, T>::value;


};

#endif /* MatrixInterface_h */
