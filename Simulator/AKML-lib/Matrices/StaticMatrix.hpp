//
//  StaticMatrix.hpp
//  AKML Project
//
//  Created by Aldric Labarthe on 17/12/2023.
//

#include "MatrixInterface.hpp"

#ifndef AKML_StaticMatrix_hpp
#define AKML_StaticMatrix_hpp

namespace akml {

template <typename element_type, std::size_t ROWS, std::size_t COLUMNS>
class StaticMatrix : public MatrixInterface<element_type>, public FSMatrixInterface{
protected:
    element_type m_data_static_array[ROWS*COLUMNS];
    
public:
    inline void create(){
        this->createInternStorage();
        for (std::size_t i(0); i < (this->rows)*(this->columns); i++){
            *(this->m_data + i) = static_cast<element_type>(0);
        }
    }
    
    inline void createInternStorage(){
        if (this->m_data == nullptr)
            this->m_data = &m_data_static_array[0];
        if (this->m_data_end == nullptr)
            this->m_data_end = this->m_data+(this->rows)*(this->columns);
    }
    
    inline void deleteInternStorage(){}
    
    inline StaticMatrix(const bool fromscratch=false) : MatrixInterface<element_type>(ROWS, COLUMNS) {
        (fromscratch) ? this->createInternStorage() : this->create();
    }
    
    inline StaticMatrix(const std::size_t rows, const std::size_t columns, const bool fromscratch=false) : MatrixInterface<element_type>(ROWS, COLUMNS) {
        if (rows != ROWS ||columns != COLUMNS)
            throw std::invalid_argument("Irregular initialization (contradictory dimension initialization).");
        (fromscratch) ? this->createInternStorage() : this->create();
    }
    
    //Column-based constructor
    inline StaticMatrix(const std::array<akml::StaticMatrix<element_type, ROWS, 1>, COLUMNS>& cols) : MatrixInterface<element_type>(ROWS, COLUMNS) {
        if (cols.size() == 0)
            throw std::invalid_argument("Empty initialize list.");
        
        this->createInternStorage();
        for (std::size_t line(0); line < this->rows; line++){
            for (std::size_t col(0); col < this->columns; col++){
                *(this->m_data + line*(this->columns)+col) = cols[col].read(line+1, 1);
            }
        }
    }
    
    inline StaticMatrix(const std::array <std::array <element_type, COLUMNS>, ROWS>& data) : MatrixInterface<element_type>(ROWS, COLUMNS) {
        this->createInternStorage();
        for (std::size_t line(0); line < ROWS; line++){
            for (std::size_t col(0); col < COLUMNS; col++){
                *(this->m_data + line*COLUMNS+col) = data[line][col];
            }
        }
    }
    
    //copy constructor
    inline StaticMatrix(const StaticMatrix<element_type, ROWS, COLUMNS>& other) : MatrixInterface<element_type>(ROWS, COLUMNS) {
        this->createInternStorage();
        std::copy(other.getStorage(), other.getStorageEnd(), this->m_data);
    }
    
    template <akml::MatrixInterfaceConcept<element_type> MatrixC>
    inline StaticMatrix(const MatrixC& other) : MatrixInterface<element_type>(ROWS, COLUMNS) {
        if (other.getNRows() != ROWS || other.getNColumns() != COLUMNS)
            throw std::invalid_argument("Error with matrix dimension.");
        this->createInternStorage();
        std::copy(other.getStorage(), other.getStorageEnd(), this->m_data);
    }
    
    inline StaticMatrix(const std::function<element_type(element_type, std::size_t, std::size_t)>& transfunc) : MatrixInterface<element_type>(ROWS, COLUMNS) {
        this->create();
        this->transform(transfunc);
    }
    
    inline StaticMatrix(const std::function<element_type(element_type)>& transfunc) : MatrixInterface<element_type>(ROWS, COLUMNS) {
        this->create();
        this->transform(transfunc);
    }
    
    inline StaticMatrix<element_type, ROWS, COLUMNS>& operator=(StaticMatrix<element_type,ROWS, COLUMNS>&& other){
        if (this != &other)
            std::copy(other.getStorage(), other.getStorageEnd(), this->m_data);
        return *this;
    }
    
    inline StaticMatrix(StaticMatrix<element_type, ROWS, COLUMNS>&& other) : MatrixInterface<element_type>(ROWS, COLUMNS) {
        createInternStorage();
        std::copy(other.getStorage(), other.getStorageEnd(), this->m_data);
    }
    
    inline void operator=(const std::array <std::array <element_type, COLUMNS>, ROWS>& data){
        for (std::size_t line(0); line < ROWS; line++){
            for (std::size_t col(0); col < COLUMNS; col++){
                *(this->getInternElement(line*COLUMNS+col)) = data[line][col];
            }
        }
    }
    
    inline StaticMatrix<element_type, ROWS, COLUMNS>& operator=(const StaticMatrix<element_type, ROWS, COLUMNS>& other){
        std::copy(other.getStorage(), other.getStorageEnd(), this->m_data);
        return *this;
    }
    
    inline friend void operator<<(std::ostream& flux, StaticMatrix<element_type, ROWS, COLUMNS> matrix){
        return StaticMatrix<element_type, ROWS, COLUMNS>::cout(matrix);
    }
    
    
    inline StaticMatrix<element_type, ROWS, COLUMNS>& operator+=(const StaticMatrix<element_type, ROWS, COLUMNS>& mat){
        for (std::size_t i(0); i < (this->rows)*(this->columns); i++){
            *(this->getInternElement(i)) += *(mat.getInternElement(i));
        }
        return *this;
    }
    
    inline friend StaticMatrix<element_type, ROWS, COLUMNS> operator+(StaticMatrix<element_type, ROWS, COLUMNS> selfmat, const StaticMatrix<element_type, ROWS, COLUMNS>& mat){
        selfmat += mat;
        return selfmat;
    }
    
    inline friend StaticMatrix<element_type, ROWS, COLUMNS> operator*(StaticMatrix<element_type, ROWS, COLUMNS> selfmat, const StaticMatrix<element_type, ROWS, COLUMNS>& mat){
        selfmat *= mat;
        return selfmat;
    }
    
    template <akml::Arithmetic NumericType>
    inline friend StaticMatrix<element_type, ROWS, COLUMNS> operator*(StaticMatrix<element_type, ROWS, COLUMNS> selfmat, const NumericType& num){
        static_assert(std::is_arithmetic<NumericType>::value, "NumericType must be numeric");
        for (element_type* tar(selfmat.getStorage()); tar < selfmat.getStorageEnd(); tar++){
            *tar = *tar * num;
        }
        return selfmat;
    }
    
    template <akml::Arithmetic NumericType>
    inline friend StaticMatrix<element_type, ROWS, COLUMNS> operator*(const NumericType& num, StaticMatrix<element_type, ROWS, COLUMNS> selfmat){
        return operator*(selfmat, num);

    }
    
    inline StaticMatrix<element_type, ROWS, COLUMNS>& operator*=(const StaticMatrix<element_type, ROWS, COLUMNS>& mat){
        *this = product(*this, mat);
        return *this;
    }
    
    inline StaticMatrix<element_type, ROWS, COLUMNS>& operator-=(const StaticMatrix<element_type, ROWS, COLUMNS>& mat){
        if (!mat.isInitialized())
            throw std::invalid_argument("Matrix provided is not initialized.");
        
        for (std::size_t i(0); i < (this->rows)*(this->columns); i++){
            *(this->getInternElement(i)) -= *(mat.getInternElement(i));
        }

        return *this;
    }
    
    inline friend StaticMatrix<element_type, ROWS, COLUMNS> operator-(StaticMatrix<element_type, ROWS, COLUMNS> selfmat, const StaticMatrix<element_type, ROWS, COLUMNS>& mat){
        selfmat -= mat;
        return selfmat;
    }
    
    
    inline void transpose(){
        if ((this->columns) != (this->rows))
            throw std::invalid_argument("Attempting to perform a direct transposition on a non-squared matrix.");
        
        element_type* localdata;
        localdata = new element_type[(this->rows)*(this->columns)];
        
        for (std::size_t i(0); i < (this->rows); i++){
            for (std::size_t j(0); j < (this->columns); j++){
                *(localdata+i*(this->rows)+j) = this->read(j+1, i+1);
            }
        }
        
        for (std::size_t i(0); i < (this->rows)*(this->columns); i++){
            m_data_static_array[i] = *(localdata+i);
        }
        
        delete[] localdata;
    }
    
    template <const std::size_t R, const std::size_t DIMTEMP, const std::size_t C>
    inline static StaticMatrix<element_type, R, C> product(const StaticMatrix<element_type, R, DIMTEMP>& A, const StaticMatrix<element_type, DIMTEMP, C>& B){
        StaticMatrix<element_type, R, C> product;
        for (std::size_t i=1; i <= A.getNRows(); i++){
            for (std::size_t j=1; j <= B.getNColumns(); j++){
                for (size_t k=1; k <= A.getNColumns(); k++){
                    product(i, j) += A.read(i, k) * B.read(k, j);
                }
            }
        }
        return product;
        
    };
};

};

#endif /* StaticMatrix_h */
