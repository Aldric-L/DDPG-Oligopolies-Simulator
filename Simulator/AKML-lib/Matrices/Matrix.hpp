//
//  Matrix.hpp
//  AKML Project
//
//  Created by Aldric Labarthe on 17/12/2023.
//

#ifndef AKML_Matrix_hpp
#define AKML_Matrix_hpp

#include "MatrixInterface.hpp"
#include "DynamicMatrix.hpp"

namespace akml {

template <typename element_type, std::size_t ROWS, std::size_t COLUMNS>
class Matrix : public DynamicMatrix<element_type>, public FSMatrixInterface{
public:
    inline void setNColumns(std::size_t& c) = delete;
    inline void setNRows(std::size_t& r) = delete;
    inline void resize(std::size_t r, std::size_t c) = delete;
    inline void directSetNColumns(std::size_t c) = delete;
    inline void directSetNRows(std::size_t r) = delete;
    template <akml::MatrixConcept MATRIX_TYPE>
    inline void forceAssignement(const MATRIX_TYPE& matrix) = delete;
    
    inline Matrix(const bool fromscratch=false) : DynamicMatrix<element_type>(ROWS, COLUMNS, fromscratch) {};
    
    inline Matrix(const std::size_t rows, const std::size_t columns, const bool fromscratch=false) : DynamicMatrix<element_type>(ROWS, COLUMNS, fromscratch) {
        if (rows != ROWS || columns != COLUMNS)
            throw std::invalid_argument("Irregular initialization (contradictory dimension initialization).");
    }
    
    //Column-based constructor
    inline Matrix(const std::array<akml::Matrix<element_type, ROWS, 1>, COLUMNS>& cols) : DynamicMatrix<element_type>(ROWS, COLUMNS, false) {
        if (cols.size() == 0)
            throw std::invalid_argument("Empty initialize list.");
        
        for (std::size_t line(0); line < this->rows; line++){
            for (std::size_t col(0); col < this->columns; col++){
                *(this->m_data + line*(this->columns)+col) = cols[col].read(line+1, 1);
            }
        }
    }
    
    inline Matrix(const std::array <std::array <element_type, COLUMNS>, ROWS>& data) : DynamicMatrix<element_type>(data) {}
    
    inline Matrix(const std::function<element_type(element_type, std::size_t, std::size_t)>& transfunc) : DynamicMatrix<element_type>(ROWS, COLUMNS, transfunc) {}
    
    inline Matrix(const std::function<element_type(element_type)>& transfunc) : DynamicMatrix<element_type>(ROWS, COLUMNS, transfunc) {}
 
    //copy constructors
    inline Matrix(const Matrix<element_type, ROWS, COLUMNS>& other) : DynamicMatrix<element_type>(ROWS, COLUMNS, true) {
        if (other.getNColumns() != COLUMNS || other.getNRows() != ROWS)
            throw std::invalid_argument("Matrix should be equally sized to be assignable.");
        
        std::copy(other.getStorage(), other.getStorageEnd(), this->m_data);
    }
    
    template <akml::MatrixInterfaceConcept<element_type> MatrixC>
    inline Matrix(const MatrixC& other) : DynamicMatrix<element_type>(ROWS, COLUMNS, true) {
        if (other.getNColumns() != COLUMNS || other.getNRows() != ROWS)
            throw std::invalid_argument("Matrix should be equally sized to be assignable.");
        
        std::copy(other.getStorage(), other.getStorageEnd(), this->m_data);
    }
    
    inline Matrix(Matrix<element_type, ROWS, COLUMNS>&& other) : DynamicMatrix<element_type>(ROWS, COLUMNS) {
        this->setMDataPointer(other.getStorage());
        other.setMDataPointer(nullptr);
    }
    
    
    template <akml::MatrixInterfaceConcept<element_type> MatrixC>
    inline Matrix<element_type, ROWS, COLUMNS>& operator=(const MatrixC& other){
        if (!this->isInitialized())
            this->createInternStorage();
        if (other.getNColumns() != COLUMNS || other.getNRows() != ROWS)
            throw std::invalid_argument("Matrix should be equally sized to be assignable.");
        
        if (other.isInitialized())
            std::copy(other.getStorage(), other.getStorageEnd(), this->m_data);
        return *this;
    }
    
    inline Matrix<element_type, ROWS, COLUMNS>& operator=(const Matrix<element_type, ROWS, COLUMNS>& other){
        if (!this->isInitialized())
            this->createInternStorage();
        
        if (other.isInitialized())
            std::copy(other.getStorage(), other.getStorageEnd(), this->m_data);
        return *this;
    }
    
    inline Matrix<element_type, ROWS, COLUMNS>& operator=(Matrix<element_type, ROWS, COLUMNS>&& other) {
        this->deleteInternStorage();
        this->setMDataPointer(other.getStorage());
        other.setMDataPointer(nullptr);
        return *this;
    }
     
    inline friend void operator<<(std::ostream& flux, Matrix<element_type, ROWS, COLUMNS> matrix){
        if (!matrix.isInitialized())
            throw std::invalid_argument("Matrix provided is not initialized.");
        
        return DynamicMatrix<element_type>::cout(matrix);
    }
    
    template <akml::MatrixInterfaceConcept<element_type> MatrixC>
    inline Matrix<element_type, ROWS, COLUMNS>& operator+=(const MatrixC& mat){
        if (!mat.isInitialized())
            throw std::invalid_argument("Matrix provided is not initialized.");
        
        if (mat.getNColumns() != (this->columns) || mat.getNRows() != (this->rows))
            throw std::invalid_argument("Matrices should be equally sized to be assignable.");
        
        for (std::size_t i(0); i < (this->rows)*(this->columns); i++){
            *(this->getInternElement(i)) += *(mat.getInternElement(i));
        }

        return *this;
    }
    
    template <akml::MatrixInterfaceConcept<element_type> MatrixC>
    inline friend Matrix<element_type, ROWS, COLUMNS> operator+(Matrix<element_type, ROWS, COLUMNS> selfmat, const MatrixC& mat){
        selfmat += mat;
        return selfmat;
    }
    
    template <akml::MatrixInterfaceConcept<element_type> MatrixC>
    inline friend Matrix<element_type, ROWS, COLUMNS> operator*(Matrix<element_type, ROWS, COLUMNS> selfmat, const MatrixC& mat){
        selfmat *= mat;
        return selfmat;
    }
    
    template <akml::Arithmetic NumericType>
    inline friend Matrix<element_type, ROWS, COLUMNS> operator*(Matrix<element_type, ROWS, COLUMNS> selfmat, const NumericType& num){
        static_assert(std::is_arithmetic<NumericType>::value, "NumericType must be numeric");
        if (selfmat.getNColumns() != COLUMNS || selfmat.getNRows() != ROWS)
            throw std::invalid_argument("Matrix should be equally sized to be assignable.");
        for (element_type* tar(selfmat.getStorage()); tar < selfmat.getStorageEnd(); tar++){
            *tar = *tar * num;
        }
        return selfmat;
    }
    
    template <akml::Arithmetic NumericType, akml::MatrixInterfaceConcept<element_type> MatrixC>
    inline friend Matrix<element_type, ROWS, COLUMNS> operator*(const NumericType& num, MatrixC selfmat){
        return operator*(selfmat, num);
    }
    
    template <akml::MatrixInterfaceConcept<element_type> MatrixC>
    inline Matrix<element_type, ROWS, COLUMNS>& operator*=(const MatrixC& mat){
        if (!mat.isInitialized())
            throw std::invalid_argument("Matrix provided is not initialized.");
        
        if (mat.getNColumns() != COLUMNS || mat.getNRows() != ROWS)
            throw std::invalid_argument("Matrix should be equally sized to be assignable.");
        
        *this = product(*this, mat);
        return *this;
    }
    
    template <akml::MatrixInterfaceConcept<element_type> MatrixC>
    inline Matrix<element_type, ROWS, COLUMNS>& operator-=(const MatrixC& mat){
        if (!mat.isInitialized())
            throw std::invalid_argument("Matrix provided is not initialized.");

        for (std::size_t i(0); i < (this->rows)*(this->columns); i++){
            *(this->getInternElement(i)) -= *(mat.getInternElement(i));
        }

        return *this;
    }
    
    template <akml::MatrixInterfaceConcept<element_type> MatrixC>
    inline friend Matrix<element_type, ROWS, COLUMNS> operator-(Matrix<element_type, ROWS, COLUMNS> selfmat, const MatrixC& mat){
        selfmat -= mat;
        return selfmat;
    }
    
    template <const std::size_t R, const std::size_t DIMTEMP, const std::size_t C>
    inline static Matrix<element_type, R, C> product(const Matrix<element_type, R, DIMTEMP>& A, const Matrix<element_type, DIMTEMP, C>& B){
        if (A.getNColumns() != B.getNRows())
            throw std::invalid_argument("Attempting to perform a non-defined matrix product.");
        if (!A.isInitialized() || !B.isInitialized())
            throw std::invalid_argument("Matrix provided is not initialized.");
        
        Matrix<element_type, R, C> product;
        for (std::size_t i=1; i <= A.getNRows(); i++){
            for (std::size_t j=1; j <= B.getNColumns(); j++){
                for (size_t k=1; k <= A.getNColumns(); k++){
                    product(i, j) += A.read(i, k) * B.read(k, j);
                }
            }
        }
        
        return product;
        
    };
    
    inline void transpose(bool force_transpose=false){
        // No fixed-size matrix should be able to force-transpose like a dynamic matrix
        DynamicMatrix<element_type>::transpose(false);
    }
};

};

#endif /* Matrix_h */
