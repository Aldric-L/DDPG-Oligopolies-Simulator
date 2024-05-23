//
//  MatrixOperations.hpp
//  AKML Project
//
//  Created by Aldric Labarthe on 20/11/2023.
//
#include <climits>
#include "MatrixInterface.hpp"
#include "StaticMatrix.hpp"
#include "DynamicMatrix.hpp"
#include "Matrix.hpp"

#ifndef AKML_MatrixOperations_hpp
#define AKML_MatrixOperations_hpp

namespace akml {
    template <akml::MatrixConcept MATRIX_TYPE, akml::Matrixable element_type>
    inline MATRIX_TYPE transform(MATRIX_TYPE matrix, const std::function<element_type(element_type, std::size_t, std::size_t)> transfunc){
        matrix.transform(transfunc);
        return matrix;
    }

    template <akml::MatrixConcept MATRIX_TYPE, akml::Matrixable element_type>
    inline MATRIX_TYPE transform(MATRIX_TYPE matrix, const std::function<element_type(element_type)> transfunc){
        matrix.transform(transfunc);
        return matrix;
    }

    
    template <akml::Matrixable element_type>
    inline DynamicMatrix<element_type> transpose(DynamicMatrix<element_type> old_matrix){
        if (!old_matrix.isInitialized())
            throw std::invalid_argument("Matrix provided is not initialized.");
        
        if (old_matrix.getNColumns() == 1 || old_matrix.getNRows() == 1){
            std::size_t old_rows = old_matrix.getNRows();
            old_matrix.directSetNRows(old_matrix.getNColumns());
            old_matrix.directSetNColumns(old_rows);
            return old_matrix;
        }
        
        element_type* localdata;
        localdata = new element_type[(old_matrix.getNRows())*(old_matrix.getNColumns())];
        for (std::size_t i=0; i < old_matrix.getNColumns(); i++){
            for (std::size_t j=0; j < old_matrix.getNRows(); j++){
                *(localdata+i*(old_matrix.getNRows())+j) = old_matrix.read(j+1, i+1);
            }
        }

        old_matrix.deleteInternStorage();
        
        old_matrix.setMDataPointer(localdata, (old_matrix.getNRows())*(old_matrix.getNColumns()));
        std::size_t old_rows = old_matrix.getNRows();
        old_matrix.directSetNRows(old_matrix.getNColumns());
        old_matrix.directSetNColumns(old_rows);
        return old_matrix;
    }

    template <akml::Matrixable element_type, std::size_t ROWS, std::size_t COLUMNS>
    inline Matrix<element_type, COLUMNS, ROWS> transpose(const Matrix<element_type, ROWS, COLUMNS>& old_matrix){
        if (!old_matrix.isInitialized())
            throw std::invalid_argument("Matrix provided is not initialized.");
        
        Matrix<element_type, COLUMNS, ROWS> newmatrix;
        if (old_matrix.getNColumns() == 1 || old_matrix.getNRows() == 1){
            newmatrix.forceByteCopy(old_matrix);
            return newmatrix;
        }
        
        for (std::size_t i=0; i < old_matrix.getNColumns(); i++){
            for (std::size_t j=0; j < old_matrix.getNRows(); j++){
                *(newmatrix.getStorage()+i*(old_matrix.getNRows())+j) = old_matrix.read(j+1, i+1);
            }
        }
        return newmatrix;
    }

    template <akml::Matrixable MATRIX_INNER_TYPE>
    inline MATRIX_INNER_TYPE inner_product(const akml::MatrixInterface<MATRIX_INNER_TYPE>& a, const akml::MatrixInterface<MATRIX_INNER_TYPE>& b){
        if (!a.isInitialized() || !b.isInitialized())
            throw std::invalid_argument("Matrix provided is not initialized.");
        
        bool A_iscol=false;
        bool B_iscol=true;
        std::size_t dim(0);
        if (a.getNRows() == 1 && b.getNColumns()==1 && a.getNColumns() == b.getNRows()){
        }else if (a.getNColumns() == 1 && b.getNRows()==1 && a.getNRows() == b.getNColumns()){
            A_iscol = true;B_iscol=false;dim=a.getNRows();
        }else if (a.getNRows() == 1 && b.getNRows()==1 && a.getNColumns() == b.getNColumns()){
            A_iscol = false;B_iscol=false;dim=a.getNColumns();
        }else if (a.getNColumns() == 1 && b.getNColumns()==1 && a.getNRows() == b.getNRows()){
            A_iscol = true;B_iscol=true;dim=a.getNRows();
        }else{
            throw std::invalid_argument("Matrices are not columns or lines");
        }
        
        MATRIX_INNER_TYPE total(0);
        for (std::size_t i=0; i <= dim; i++){
            total += (A_iscol ? a.read(i, 1) : a.read(1, i)) * (B_iscol ? b.read(i, 1) : b.read(1, i));
        }
        return total;
    }

    template <akml::Matrixable MATRIX_INNER_TYPE>
    inline MATRIX_INNER_TYPE inner_product(const akml::MatrixInterface<MATRIX_INNER_TYPE>& a){
        if (!a.isInitialized())
            throw std::invalid_argument("Matrix provided is not initialized.");
        
        bool A_iscol=false;
        std::size_t dim(0);
        if (a.getNColumns() == 1){
            A_iscol = true;dim=a.getNRows();
        }else if (a.getNRows() == 1){
            A_iscol = false;dim=a.getNColumns();
        }else{
            throw std::invalid_argument("Matrices are not columns or lines");
        }
        
        MATRIX_INNER_TYPE total(0);
        for (std::size_t i=1; i <= dim; i++){
            total += (A_iscol ? a.read(i, 1) : a.read(1, i));
        }
        return total;
    }

    template <akml::MatrixConcept MATRIX_TYPE>
    inline MATRIX_TYPE hadamard_product(MATRIX_TYPE A, MATRIX_TYPE B){
        if (!A.isInitialized() || !B.isInitialized())
            throw std::invalid_argument("Matrix provided is not initialized.");
        if (A.getNColumns() != B.getNColumns() || A.getNRows() != B.getNRows())
            throw std::invalid_argument("Attempting to perform a product on non-equally sized matrix.");
        
        MATRIX_TYPE product(A.getNRows(), A.getNColumns());
        for (std::size_t i=1; i <= A.getNRows(); i++){
            for (std::size_t j=1; j <= B.getNColumns(); j++){
                product(i, j) = A(i, j) * B(i, j);
            }
        }
        return product;
    };

    template <akml::MatrixConcept MATRIX_TYPE>
    inline MATRIX_TYPE hadamard_division(MATRIX_TYPE A, MATRIX_TYPE B){
        if (!A.isInitialized() || !B.isInitialized())
            throw std::invalid_argument("Matrix provided is not initialized.");
        if (A.getNColumns() != B.getNColumns() || A.getNRows() != B.getNRows())
            throw std::invalid_argument("Attempting to perform a product on non-equally sized matrix.");
        
        MATRIX_TYPE product(A.getNRows(), A.getNColumns());
        for (std::size_t i=1; i <= A.getNRows(); i++){
            for (std::size_t j=1; j <= B.getNColumns(); j++){
                product(i, j) = A(i, j) * (1/B(i, j));
            }
        }
        return product;
    };

    template <akml::MatrixConcept MATRIX_TYPE>
    inline MATRIX_TYPE matrix_pow(MATRIX_TYPE A, const unsigned int power){
        if (!A.isInitialized())
            throw std::invalid_argument("Matrix provided is not initialized.");
        if (A.getNColumns() != A.getNRows() )
            throw std::invalid_argument("Attempting to pow non-squared matrix.");
        
        for (std::size_t i=2; i <= power; i++){
            A *= std::move(A);
        }
        return A;
    };

    template <akml::Matrixable MATRIX_INNER_TYPE>
    inline MATRIX_INNER_TYPE sum_column(const akml::MatrixInterface<MATRIX_INNER_TYPE>& A, std::size_t col=1){
        if (!A.isInitialized())
            throw std::invalid_argument("Matrix provided is not initialized.");
        if (A.getNColumns() < col)
            throw std::invalid_argument("Attempting to perform a product on non-equally sized matrix.");
        MATRIX_INNER_TYPE result(0);
        for (std::size_t i=1; i <= A.getNRows(); i++){
            result += A.read(i, col);
        }
        return result;
    };

    template <akml::Matrixable element_type>
    inline DynamicMatrix<element_type> matrix_product(const DynamicMatrix<element_type>& A, const DynamicMatrix<element_type>& B){
        return DynamicMatrix<element_type>::product(A, B);
    };

    template <akml::Matrixable element_type, std::size_t ROWS, std::size_t TEMPDIM, std::size_t COLUMNS>
    inline StaticMatrix<element_type,ROWS, COLUMNS> matrix_product(const StaticMatrix<element_type, ROWS, TEMPDIM>& A, const StaticMatrix<element_type, TEMPDIM, COLUMNS>& B){
        return StaticMatrix<element_type, ROWS, COLUMNS>::product(A, B);
    };

    template <akml::Matrixable element_type, std::size_t ROWS, std::size_t TEMPDIM, std::size_t COLUMNS>
    inline Matrix<element_type,ROWS, COLUMNS> matrix_product(const Matrix<element_type, ROWS, TEMPDIM>& A, const Matrix<element_type, TEMPDIM, COLUMNS>& B){
        return Matrix<element_type, ROWS, COLUMNS>::product(A, B);
    };


    template <akml::Matrixable element_type>
    inline void cout_matrix(const MatrixInterface<element_type>& A){
        return MatrixInterface<element_type>::cout(A);
        
    };

    template <akml::Arithmetic element_type>
    inline element_type abs(element_type& val){ return std::abs(val); }

    template<>
    inline std::size_t abs(std::size_t& val){ return val; }

    template <akml::Arithmetic element_type>
    inline std::size_t arg_max(const element_type* begin, const element_type* end) {
        return static_cast<std::size_t>(std::distance(begin, std::max_element(begin, end)));
    }

    template <akml::Arithmetic element_type>
    inline std::size_t arg_min(const element_type* begin, const element_type* end) {
        return static_cast<std::size_t>(std::distance(begin, std::min_element(begin, end)));
    }

    template <akml::Arithmetic element_type>
    inline std::size_t arg_max(const MatrixInterface<element_type>& matrix, const bool absval_mode=false) {
        if (!matrix.isInitialized())
            throw std::invalid_argument("Matrix provided is not initialized.");
        
        if (matrix.getNRows() != 1 && matrix.getNColumns() != 1)
            throw std::invalid_argument("Arg_max only applies on column vectors, not on every matrix.");
        
        if (absval_mode){
            element_type* newstorage = new element_type[matrix.getNRows() * matrix.getNColumns()];
            for (std::size_t i(0); i < matrix.getNRows() * matrix.getNColumns(); i++){
                *(newstorage+i) = akml::abs(*(matrix.getStorage()+i));
            }
            std::size_t result = arg_max(newstorage, newstorage+matrix.getNRows() * matrix.getNColumns());
            delete[] newstorage;
            return result;
        }
        return arg_max(matrix.getStorage(), matrix.getStorageEnd());
    }

    template <akml::Arithmetic element_type>
    inline std::size_t arg_min(const MatrixInterface<element_type>& matrix, const bool absval_mode=false) {
        if (!matrix.isInitialized())
            throw std::invalid_argument("Matrix provided is not initialized.");
        
        if (matrix.getNRows() != 1 && matrix.getNColumns() != 1)
            throw std::invalid_argument("Arg_min only applies on column vectors, not on every matrix.");
        
        if (absval_mode){
            element_type* newstorage = new element_type[matrix.getNRows() * matrix.getNColumns()];
            for (std::size_t i(0); i < matrix.getNRows() * matrix.getNColumns(); i++){
                *(newstorage+i) = akml::abs(*(matrix.getStorage()+i));
            }
            std::size_t result = arg_min(newstorage, newstorage+matrix.getNRows() * matrix.getNColumns());
            delete[] newstorage;
            return result;
        }
        
        return arg_min(matrix.getStorage(), matrix.getStorageEnd());
    }

    template <akml::Arithmetic element_type>
    inline element_type max(const element_type* begin, const element_type* end) {
        return *std::max_element(begin, end);
    }

    template <akml::Arithmetic element_type>
    inline element_type min(const element_type* begin, const element_type* end) {
        return *std::min_element(begin, end);
    }

    template <akml::Arithmetic element_type>
    inline element_type max(const MatrixInterface<element_type>& matrix) {
        if (!matrix.isInitialized())
            throw std::invalid_argument("Matrix provided is not initialized.");
        
        if (matrix.getNRows() != 1 && matrix.getNColumns() != 1)
            throw std::invalid_argument("Max only applies on column vectors, not on every matrix.");
        
        return max(matrix.getStorage(), matrix.getStorageEnd());
    }

    template <akml::Arithmetic element_type>
    inline element_type min(const MatrixInterface<element_type>& matrix, const bool absval_mode=false) {
        if (!matrix.isInitialized())
            throw std::invalid_argument("Matrix provided is not initialized.");
        
        if (matrix.getNRows() != 1 && matrix.getNColumns() != 1)
            throw std::invalid_argument("Min only applies on column vectors, not on every matrix.");

        return min(matrix.getStorage(), matrix.getStorageEnd());
    }

    template <akml::Arithmetic element_type>
    inline float mean(const MatrixInterface<element_type>& matrix, const bool absval_mode=false, const element_type ignore=static_cast<element_type>(1)) {
        if (!matrix.isInitialized())
            throw std::invalid_argument("Matrix provided is not initialized.");
        
        if (matrix.getNRows() != 1 && matrix.getNColumns() != 1)
            throw std::invalid_argument("Mean only applies on column vectors, not on every matrix.");
        
        float mean = 0.f;
        for (std::size_t i(0); i < matrix.getNRows() * matrix.getNColumns(); i++){
            if (ignore == static_cast<element_type>(1) || *(matrix.getStorage()+i) != ignore)
                mean += absval_mode ? akml::abs(*(matrix.getStorage()+i)) : *(matrix.getStorage()+i);
                //mean += absval_mode ? std::abs(*(matrix.getStorage()+i)) : *(matrix.getStorage()+i);

        }
        mean = mean / std::max(matrix.getNColumns(),matrix.getNRows());
        return mean;
    }

    template <akml::MatrixConcept MATRIX_TYPE>
    inline MATRIX_TYPE mean(const std::vector<MATRIX_TYPE>& matrices){
        if (matrices.size() == 0)
            throw std::invalid_argument("Empty vector of matrices provided.");
        
        MATRIX_TYPE mean(matrices.front().getNRows(), matrices.front().getNColumns());
        for (std::size_t mat_i(0); mat_i < matrices.size(); mat_i++){
            if (!matrices.at(mat_i).isInitialized())
                throw std::invalid_argument("Matrix provided is not initialized.");
            
            if (matrices.at(mat_i).getNColumns() != mean.getNColumns() || matrices.at(mat_i).getNRows() != mean.getNRows())
                throw std::invalid_argument("Attempting to perform a mean on non-equally sized matrix.");
            
            for (std::size_t i=1; i <= matrices.at(mat_i).getNRows(); i++){
                for (std::size_t j=1; j <= matrices.at(mat_i).getNColumns(); j++){
                    mean(i, j) += matrices.at(mat_i).read(i, j);
                }
            }
        }
        for (std::size_t i=1; i <= mean.getNRows(); i++){
            for (std::size_t j=1; j <= mean.getNColumns(); j++){
                mean(i, j) = mean(i, j)/matrices.size();
            }
        }
        return mean;
    };

    template <akml::Arithmetic element_type>
    inline float stat_var(const MatrixInterface<element_type>& matrix, const element_type ignore=static_cast<element_type>(1),  float mean=0) {
        if (!matrix.isInitialized())
            throw std::invalid_argument("Matrix provided is not initialized.");
        
        if (matrix.getNRows() != 1 && matrix.getNColumns() != 1)
            throw std::invalid_argument("Variance only applies on column vectors, not on every matrix.");
        
        if (mean == 0){
            for (std::size_t i(0); i < matrix.getNRows() * matrix.getNColumns(); i++){
                if (ignore == static_cast<element_type>(1) || *(matrix.getStorage()+i) != ignore)
                    mean += *(matrix.getStorage()+i);
            }
            mean = mean / std::max(matrix.getNColumns(),matrix.getNRows());
        }
        float variance = 0.f;
        for (std::size_t i(0); i < matrix.getNRows() * matrix.getNColumns(); i++){
            if (ignore == static_cast<element_type>(1) || *(matrix.getStorage()+i) != ignore)
                variance += std::pow(mean - *(matrix.getStorage()+i), 2);
        }
        variance = variance / std::max(matrix.getNColumns(),matrix.getNRows());
        return variance;
    }

    /*
     * __localComputeDijkstra - THIS METHOD SHOULD NEVER BE USED
     * It has been created to avoid code duplication due to matrix types.
     * Please use the dijkstra_distance_algorithm who does all the verifications for you
     * Freely adjusted from an article of geeksforgeeks.org
     */
    template <akml::Arithmetic element_type>
    inline void __localComputeDijkstra(const MatrixInterface<element_type>& matrix, const std::size_t& from, MatrixInterface<std::size_t>* dist){
        akml::DynamicMatrix<element_type> shortestPathTreeSet(matrix.getNColumns(), 1);
     
        for (std::size_t i = 0; i < matrix.getNColumns(); i++){
            (*dist)[{i, 0}] = ULONG_MAX;
            shortestPathTreeSet[{i,0}] = false;
        }
             
        (*dist)[{from, 0}] = 0;
     
        for (std::size_t count = 0; count < matrix.getNColumns() - 1; count++) {
            std::size_t u = ULONG_MAX, min = ULONG_MAX;
         
            for (std::size_t v = 0; v < matrix.getNColumns(); v++){
                if (shortestPathTreeSet[{v, 0}] == false && (*dist)[{v, 0}] <= min){
                    min = (*dist)[{v, 0}];
                    u = v;
                }
            }
            if (u != ULONG_MAX){
                shortestPathTreeSet[{u, 0}] = true;
         
                for (std::size_t v = 0; v < matrix.getNColumns(); v++)
                    if (!shortestPathTreeSet[{v, 0}] && matrix[{u, v}] > 0
                        && (*dist)[{u, 0}] != ULONG_MAX
                        && (*dist)[{u, 0}] + matrix[{u, v}] < (*dist)[{v, 0}])
                        (*dist)[{v, 0}] = (*dist)[{u, 0}] + matrix[{u, v}];
            }
        }
    }

    /*
     * dijkstra_distance_algorithm - Method to find the length of the shortest path between two vertices
     */
    template <akml::Arithmetic element_type, std::size_t MATRIX_DIM>
    inline akml::Matrix<std::size_t, MATRIX_DIM, 1> dijkstra_distance_algorithm(const Matrix<element_type, MATRIX_DIM, MATRIX_DIM>& matrix, const std::size_t from) {
        if (!matrix.isInitialized())
            throw std::invalid_argument("Matrix provided is not initialized.");
        
        if (matrix.getNRows() <= 1 || matrix.getNColumns() != matrix.getNRows() || matrix.getNColumns() < from )
            throw std::invalid_argument("Error with matrix dimension.");
        
        akml::Matrix<std::size_t, MATRIX_DIM, 1> dist;
        __localComputeDijkstra(matrix, from, &dist);
        return dist;
    }

    template <akml::Arithmetic element_type>
    inline akml::DynamicMatrix<std::size_t> dijkstra_distance_algorithm(const DynamicMatrix<element_type>& matrix, const std::size_t from) {
        if (!matrix.isInitialized())
            throw std::invalid_argument("Matrix provided is not initialized.");
        
        if (matrix.getNRows() <= 1 || matrix.getNColumns() != matrix.getNRows() || matrix.getNColumns() < from )
            throw std::invalid_argument("Error with matrix dimension.");
        
        akml::DynamicMatrix<std::size_t> dist(matrix.getNColumns(), 1);
        __localComputeDijkstra(matrix, from, &dist);
        return dist;
    }

    template <akml::Arithmetic element_type, std::size_t MATRIX_DIM>
    inline akml::StaticMatrix<std::size_t, MATRIX_DIM, 1> dijkstra_distance_algorithm(const StaticMatrix<element_type, MATRIX_DIM, MATRIX_DIM>& matrix, const std::size_t from) {
        if (!matrix.isInitialized())
            throw std::invalid_argument("Matrix provided is not initialized.");
        
        if (matrix.getNRows() <= 1 || matrix.getNColumns() != matrix.getNRows() || matrix.getNColumns() < from )
            throw std::invalid_argument("Error with matrix dimension.");
        
        akml::StaticMatrix<std::size_t, MATRIX_DIM, 1> dist;
        __localComputeDijkstra(matrix, from, &dist);
        return dist;
    }
    
    template<akml::Matrixable MATRIX_INNER_TYPE, akml::Matrixable... MATRIX_INNER_TYPES> requires (std::same_as<MATRIX_INNER_TYPE, MATRIX_INNER_TYPES> && ...)
    inline akml::Matrix<MATRIX_INNER_TYPE, sizeof...(MATRIX_INNER_TYPES), 1> make_vector(const MATRIX_INNER_TYPES... elements) {
        
        akml::Matrix<MATRIX_INNER_TYPE, sizeof...(MATRIX_INNER_TYPES), 1> newmat((std::array<std::array<MATRIX_INNER_TYPE, 1>, sizeof...(MATRIX_INNER_TYPES)>){{ {elements}... }});
        return newmat;
    }

    template<akml::Matrixable MATRIX_INNER_TYPE, akml::Matrixable... MATRIX_INNER_TYPES> requires (std::same_as<MATRIX_INNER_TYPE, MATRIX_INNER_TYPES> && ...)
    inline DynamicMatrix<MATRIX_INNER_TYPE> make_dynamic_vector(const MATRIX_INNER_TYPES... elements) {
        akml::DynamicMatrix<MATRIX_INNER_TYPE> newmat((std::array<std::array<MATRIX_INNER_TYPE, 1>, sizeof...(MATRIX_INNER_TYPES)>){{ {elements}... }});
        return newmat;
    }

    template<akml::Matrixable MATRIX_INNER_TYPE>
    inline akml::DynamicMatrix<MATRIX_INNER_TYPE> make_diagonal(const akml::DynamicMatrix<MATRIX_INNER_TYPE>& mat) {
        if (!mat.isInitialized())
            throw std::invalid_argument("Matrix provided is not initialized.");
        if (mat.getNRows() > 1 && mat.getNColumns() > 1)
            throw std::invalid_argument("Error with matrix dimension.");
            
        akml::DynamicMatrix<MATRIX_INNER_TYPE> res ((mat.getNRows() == 1) ? mat.getNColumns() : mat.getNRows(), (mat.getNRows() == 1) ? mat.getNColumns() : mat.getNRows());
        for (std::size_t row(0); row < std::max(mat.getNRows(), mat.getNColumns()); row++){
            res[{row, row}] = (mat.getNRows() == 1) ?  mat[{0, row}] : mat[{row, 0}];
        }
        return res;
    }

    template <akml::MatrixConcept MATRIX_TYPE>
    inline MATRIX_TYPE& make_identity(MATRIX_TYPE& A){
        if (!A.isInitialized())
            throw std::invalid_argument("Matrix provided is not initialized.");
        if (A.getNColumns() != A.getNRows())
            throw std::invalid_argument("Identity matrix is a squared matrix.");
        
        for (std::size_t row(0); row < A.getNRows(); row++){
            for (std::size_t col(0); col < A.getNColumns(); col++){
                A[{row, col}] = (row == col) ? 1 : 0;
            }
        }
        return A;
    };

    template <akml::MatrixConcept MATRIX_TYPE>
    inline MATRIX_TYPE make_identity(const std::size_t dim){
        MATRIX_TYPE A (dim, dim);
        for (std::size_t row(0); row < dim; row++){
            A[{row, row}] = 1;
        }
        return A;
    };

}
#endif /* MatrixOperations_h */
