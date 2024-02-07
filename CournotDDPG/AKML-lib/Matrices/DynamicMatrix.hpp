//
//  DynamicMatrix.hpp
//  AKML Project
//
//  Created by Aldric Labarthe on 17/12/2023.
//

#include "MatrixInterface.hpp"

#ifndef AKML_DynamicMatrix_hpp
#define AKML_DynamicMatrix_hpp

namespace akml {

template <typename element_type, std::size_t ROWS, std::size_t COLUMNS>
class Matrix;

template <typename element_type>
class DynamicMatrix : public MatrixInterface<element_type> {
public:
    inline void setNColumns(std::size_t c) { resize(this->rows, c); }
    inline void setNRows(std::size_t r) { resize(r, this->columns); }
    inline void directSetNColumns(std::size_t c) { this->columns = c; }
    inline void directSetNRows(std::size_t r) { this->rows = r; }
    
    inline void resize(std::size_t r, std::size_t c){
        if (this->isInitialized()){
            if (c == this->getNColumns() && r == this->getNRows())
                return;
            
            element_type* newstorage = new element_type[r*c];
            for (std::size_t line(0); line < r; line++){
                for (std::size_t col(0); col < c; col++){
                    if (col >= this->getNColumns() || line >= this->getNRows())
                        *(newstorage + line*(c)+col) = static_cast<element_type>(0);
                    else
                        *(newstorage + line*(c)+col) = this->operator[]({line, col});
                }
            }
            delete[] this->m_data;
            this->directSetNColumns(c);
            this->directSetNRows(r);
            this->setMDataPointer(newstorage);
        }else {
            this->directSetNColumns(c);
            this->directSetNRows(r);
        }
    }
    
    // This method is deprecated as we use default values provided in new type().
    // No need, hence, to manually set each value
    inline void create(){ this->createInternStorage(); }
    
    inline void createInternStorage(){
        if (this->m_data != nullptr || this->m_data_end != nullptr)
            this->deleteInternStorage();
        
        this->m_data = new element_type[(this->rows)*(this->columns)]();
        this->m_data_end = this->m_data+(this->rows)*(this->columns);
    }
    
    inline void deleteInternStorage(){
        if (this->m_data != nullptr)
            delete[] this->m_data;
        this->m_data = nullptr;
        this->m_data_end = nullptr;
    }
    
    inline DynamicMatrix(const std::size_t rows, const std::size_t columns, const bool fromscratch=false) : MatrixInterface<element_type>(rows, columns) {
        (fromscratch) ? this->createInternStorage() : this->create();
    }
    
    //Column-based constructor
    template<std::size_t COLUMNS>
    inline DynamicMatrix(const std::array<akml::DynamicMatrix<element_type>, COLUMNS>& cols) : MatrixInterface<element_type>(cols[0].getNRows(), COLUMNS) {
        if (cols.size() == 0)
            throw std::invalid_argument("Empty initialize list.");
        
        this->createInternStorage();
        for (std::size_t line(0); line < this->rows; line++){
            for (std::size_t col(0); col < this->columns; col++){
                *(this->m_data + line*(this->columns)+col) = cols[col].read(line+1, 1);
            }
        }
    }
    
    //Column-based constructor
    template <akml::MatrixInterfaceConcept<element_type> MatrixC>
    inline DynamicMatrix(const std::vector<MatrixC>& cols) : MatrixInterface<element_type>(cols[0].getNRows(), cols.size()) {
        if (cols.size() == 0)
            throw std::invalid_argument("Empty initialize list.");
        
        for (std::size_t col(0); col < this->columns; col++){
            if (cols.at(col).getNRows() != this->rows)
                throw std::invalid_argument("Heterogeneous initialize list.");
        }
        
        this->createInternStorage();
        for (std::size_t line(0); line < this->rows; line++){
            for (std::size_t col(0); col < this->columns; col++){
                *(this->m_data + line*(this->columns)+col) = cols.at(col).read(line+1, 1);
            }
        }
    }
    
    //Column-based constructor
    template <akml::MatrixInterfaceConcept<element_type> MatrixC>
    inline DynamicMatrix(const std::vector<MatrixC*>& cols) : MatrixInterface<element_type>(cols[0]->getNRows(), cols.size()) {
        if (cols.size() == 0)
            throw std::invalid_argument("Empty initialize list.");
        
        for (std::size_t col(0); col < this->columns; col++){
            if (cols.at(col)->getNRows() != this->rows)
                throw std::invalid_argument("Heterogeneous initialize list.");
        }
        
        this->createInternStorage();
        for (std::size_t line(0); line < this->rows; line++){
            for (std::size_t col(0); col < this->columns; col++){
                *(this->m_data + line*(this->columns)+col) = cols.at(col)->read(line+1, 1);
            }
        }
    }
    
    template<std::size_t ROWS>
    inline DynamicMatrix(const std::vector<akml::Matrix<element_type, ROWS, 1>>& cols) : MatrixInterface<element_type>(ROWS, cols.size()) {
        if (cols.size() == 0)
            throw std::invalid_argument("Empty initialize list.");
        
        this->createInternStorage();
        for (std::size_t line(0); line < this->rows; line++){
            for (std::size_t col(0); col < this->columns; col++){
                *(this->m_data + line*(this->columns)+col) = cols[col].read(line+1, 1);
            }
        }
    }
    
    template<std::size_t ROWS, std::size_t COLUMNS>
    inline DynamicMatrix(const std::array <std::array <element_type, COLUMNS>, ROWS>& data) : MatrixInterface<element_type>(ROWS, COLUMNS) {
        this->createInternStorage();
        for (std::size_t line(0); line < ROWS; line++){
            for (std::size_t col(0); col < COLUMNS; col++){
                *(this->m_data + line*COLUMNS+col) = data[line][col];
            }
        }
    }
    
    inline DynamicMatrix(const std::size_t rows, const std::size_t columns, const std::function<element_type(element_type, std::size_t, std::size_t)>& transfunc) : MatrixInterface<element_type>(rows, columns) {
        this->create();
        this->transform(transfunc);
    }
    
    inline DynamicMatrix(const std::size_t rows, const std::size_t columns, const std::function<element_type(element_type)>& transfunc) : MatrixInterface<element_type>(rows, columns) {
        this->create();
        this->transform(transfunc);
    }
    
    //copy constructors
    inline DynamicMatrix(const DynamicMatrix<element_type>& other) : MatrixInterface<element_type>(other.getNRows(), other.getNColumns()) {
        this->createInternStorage();
        std::copy(other.getStorage(), other.getStorageEnd(), this->m_data);
    }
    
    template <akml::MatrixInterfaceConcept<element_type> MatrixC>
    inline DynamicMatrix(const MatrixC& other) : MatrixInterface<element_type>(other.getNRows(), other.getNColumns()) {
        this->createInternStorage();
        std::copy(other.getStorage(), other.getStorageEnd(), this->m_data);
    }
    
    inline DynamicMatrix(DynamicMatrix<element_type>&& other) : MatrixInterface<element_type>(other.getNRows(), other.getNColumns()) {
        this->setMDataPointer(other.getStorage());
        other.setMDataPointer(nullptr);
    }

    inline ~DynamicMatrix(){
        this->deleteInternStorage();
    }
    
    template <akml::MatrixConcept MATRIX_TYPE>
    inline void forceAssignement(const MATRIX_TYPE& matrix){
        this->resize(matrix.getNRows(), matrix.getNColumns());
        this->operator=(matrix);
    }
    
    inline void forceByteCopy(const element_type* storage, std::size_t len=0){
        std::copy(storage, storage + ((len == 0) ? (this->rows)*(this->columns) : len), this->m_data);
    }
    
    inline void forceByteCopy(const MatrixInterface<element_type>& target){
        std::copy(target.getStorage(), target.getStorageEnd(), this->m_data);
    }
    
    template<std::size_t ROWS, std::size_t COLUMNS>
    inline void operator=(const std::array <std::array <element_type, COLUMNS>, ROWS>& data){
        if (COLUMNS != (this->columns) || ROWS != (this->rows))
            throw std::invalid_argument("Array and Matrix should be equally sized to be assignable.");
        
        if (!this->isInitialized())
            this->createInternStorage();
        
        for (std::size_t line(0); line < ROWS; line++){
            for (std::size_t col(0); col < COLUMNS; col++){
                *(this->getInternElement(line*COLUMNS+col)) = data[line][col];
            }
        }
    }
    
    inline DynamicMatrix<element_type>& operator=(const DynamicMatrix<element_type>& other){
        //if (this != &other){
            if (other.getNColumns() != (this->columns) || other.getNRows() != (this->rows))
                throw std::invalid_argument("Matrices should be equally sized to be assignable.");
            
            if (!other.isInitialized())
                throw std::invalid_argument("DynamicMatrix provided is not initialized.");
            
            if (!this->isInitialized())
                this->createInternStorage();
            
            std::copy(other.getStorage(), other.getStorageEnd(), this->m_data);
        //}
        
        return *this;
    }
    
    template <akml::MatrixInterfaceConcept<element_type> MatrixC>
    //inline DynamicMatrix<element_type>& operator=(const DynamicMatrix<element_type>& other){
    inline DynamicMatrix<element_type>& operator=(const MatrixC& other){
        //if (this != &other){
            if (other.getNColumns() != (this->columns) || other.getNRows() != (this->rows))
                throw std::invalid_argument("Matrices should be equally sized to be assignable.");
            
            if (!other.isInitialized())
                throw std::invalid_argument("DynamicMatrix provided is not initialized.");
            
            if (!this->isInitialized())
                this->createInternStorage();
            
            std::copy(other.getStorage(), other.getStorageEnd(), this->m_data);
        //}
        
        return *this;
    }
    
    template <akml::MatrixInterfaceConcept<element_type> MatrixC>
    //inline DynamicMatrix<element_type>& operator=(const DynamicMatrix<element_type>& other){
    inline DynamicMatrix<element_type>& operator=(const MatrixC&& other){
        //if (this != &other){
            if (other.getNColumns() != (this->columns) || other.getNRows() != (this->rows))
                throw std::invalid_argument("Matrices should be equally sized to be assignable.");
            
            if (!other.isInitialized())
                throw std::invalid_argument("DynamicMatrix provided is not initialized.");
            
            if (!this->isInitialized())
                this->createInternStorage();
            
            std::copy(other.getStorage(), other.getStorageEnd(), this->m_data);
        //}
        
        return *this;
    }
    
    inline DynamicMatrix<element_type>& operator=(DynamicMatrix<element_type>&& other){
        if (this != &other) {
            if (other.getNColumns() != this->columns || other.getNRows() != this->rows)
                throw std::invalid_argument("Matrix should be equally sized to be assignable.");
            this->deleteInternStorage();
            this->setMDataPointer(other.getStorage());
            other.setMDataPointer(nullptr);
        }
        return *this;
    }
    
    inline friend void operator<<(std::ostream& flux, DynamicMatrix<element_type> matrix){
        if (!matrix.isInitialized())
            throw std::invalid_argument("Matrix provided is not initialized.");
        
        return DynamicMatrix<element_type>::cout(matrix);
    }
    
    template <akml::MatrixInterfaceConcept<element_type> MatrixC>
    inline DynamicMatrix<element_type>& operator+=(const MatrixC& mat){
    //inline DynamicMatrix<element_type>& operator+=(const DynamicMatrix<element_type>& mat){
        if (mat.getNColumns() != (this->columns) || mat.getNRows() != (this->rows))
            throw std::invalid_argument("Matrices should be equally sized to be assignable.");
        
        if (!mat.isInitialized())
            throw std::invalid_argument("Matrix provided is not initialized.");
        
        for (std::size_t i(0); i < (this->rows)*(this->columns); i++){
            *(this->getInternElement(i)) += *(mat.getInternElement(i));
        }
        return *this;
    }
    
    template <akml::MatrixInterfaceConcept<element_type> MatrixC>
    inline friend DynamicMatrix<element_type> operator+(DynamicMatrix<element_type> selfmat, const MatrixC& mat){
    //inline friend DynamicMatrix<element_type> operator+(DynamicMatrix<element_type> selfmat, const DynamicMatrix<element_type>& mat){
        selfmat += mat;
        return selfmat;
    }
    
    template <akml::MatrixInterfaceConcept<element_type> MatrixC>
    inline friend DynamicMatrix<element_type> operator*(DynamicMatrix<element_type> selfmat, const MatrixC& mat){
        selfmat *= mat;
        return selfmat;
    }
    
    template <akml::Arithmetic NumericType>
    inline friend DynamicMatrix<element_type> operator*(const DynamicMatrix<element_type> selfmat, const NumericType& num){
        for (element_type* tar(selfmat.getStorage()); tar < selfmat.getStorageEnd(); tar++){
            *tar = *tar * num;
        }
        return selfmat;
    }
    
    template <akml::Arithmetic NumericType>
    inline friend DynamicMatrix<element_type> operator*(const NumericType& num, const DynamicMatrix<element_type>& selfmat){
        return operator*(selfmat, num);
    }
    
    template <akml::MatrixInterfaceConcept<element_type> MatrixC>
    inline DynamicMatrix<element_type>& operator*=(const MatrixC& mat){
        if (!mat.isInitialized())
            throw std::invalid_argument("Matrix provided is not initialized.");
        if (mat.getNColumns() != (this->columns) || mat.getNRows() != (this->rows))
            throw std::invalid_argument("Matrices should be equally sized to be assignable.");
        
        this->forceByteCopy(product(*this, mat).getStorage());
        return *this;
    }
    
    template <akml::MatrixInterfaceConcept<element_type> MatrixC>
    inline DynamicMatrix<element_type>& operator-=(const MatrixC& mat){
        if (!mat.isInitialized())
            throw std::invalid_argument("Matrix provided is not initialized.");
        
        if (mat.getNColumns() != (this->columns) || mat.getNRows() != (this->rows))
            throw std::invalid_argument("Matrices should be equally sized to be assignable.");
        
        for (std::size_t i(0); i < (this->rows)*(this->columns); i++){
            *(this->getInternElement(i)) -= *(mat.getInternElement(i));
        }

        return *this;
    }
    
    template <akml::MatrixInterfaceConcept<element_type> MatrixC>
    inline friend DynamicMatrix<element_type> operator-(DynamicMatrix<element_type> selfmat, const MatrixC& mat){
        selfmat -= mat;
        return selfmat;
    }
    
    
    inline void transpose(bool force_transpose=false){
        if (!force_transpose && (this->columns) != (this->rows))
            throw std::invalid_argument("Attempting to perform a direct transposition on a non-squared matrix.");
        
        if ((this->columns) == 1 || (this->rows) == 1){
            std::size_t r = this->getNRows();
            this->directSetNRows(this->getNColumns());
            this->directSetNColumns(r);
            return;
        }
        
        element_type* localdata;
        localdata = new element_type[(this->rows)*(this->columns)];
        
        for (std::size_t i=0; i < this->getNColumns(); i++){
            for (std::size_t j=0; j < this->getNRows(); j++){
                *(localdata+i*(this->getNRows())+j) = this->read(j+1, i+1);
            }
        }
        
        delete[] this->m_data;
        this->setMDataPointer(localdata);
        
        if (force_transpose){
            std::size_t r = this->getNRows();
            this->directSetNRows(this->getNColumns());
            this->directSetNColumns(r);
        }
    }
    
    inline static DynamicMatrix<element_type> product(const DynamicMatrix<element_type>& A, const DynamicMatrix<element_type>& B){
        if (A.getNColumns() != B.getNRows())
            throw std::invalid_argument("Attempting to perform a non-defined matrix product.");
        if (!A.isInitialized() || !B.isInitialized())
            throw std::invalid_argument("Matrix provided is not initialized.");
        
        DynamicMatrix<element_type> product(A.getNRows(), B.getNColumns());
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


#endif /* DynamicMatrix_h */
