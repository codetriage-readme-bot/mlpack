/**
 * @file fft_convolution.hpp
 * @author Shangtong Zhang
 * @author Marcus Edel
 *
 * Implementation of the convolution through fft.
 */
#ifndef __MLPACK_METHODS_ANN_CONVOLUTION_RULES_FFT_CONVOLUTION_HPP
#define __MLPACK_METHODS_ANN_CONVOLUTION_RULES_FFT_CONVOLUTION_HPP

#include <mlpack/core.hpp>
#include "border_modes.hpp"

namespace mlpack {
namespace ann /** Artificial Neural Network. */ {

/**
 * Computes the two-dimensional convolution through fft. This class allows
 * specification of the type of the border type. The convolution can be compute
 * with the valid border type of the full border type (default).
 *
 * FullConvolution: returns the full two-dimensional convolution.
 * ValidConvolution: returns only those parts of the convolution that are
 * computed without the zero-padded edges.
 *
 * @tparam BorderMode Type of the border mode (FullConvolution or
 * ValidConvolution).
 * @tparam padLastDim Pad the last dimension of the input to to turn it from
 * odd to even.
 */
template<typename BorderMode = FullConvolution, const bool padLastDim = false>
class FFTConvolution
{
 public:
  /*
   * Perform a convolution through fft (valid mode). This method only supports
   * input which is even on the last dimension. In case of an odd input with, a
   * user can manually pad the imput or specify the padLastDim parameter which
   * takes care of padding. The filter instead can have any size. When using the
   * valid mode the filters has to be smaller than the input.
   *
   * @param input Input used to perform the convolution.
   * @param filter Filter used to perform the conolution.
   * @param output Output data that contains the results of the convolution.
   */
  template<typename eT, typename Border = BorderMode>
  static typename std::enable_if<
      std::is_same<Border, ValidConvolution>::value, void>::type
  Convolution(const arma::Mat<eT>& input,
              const arma::Mat<eT>& filter,
              arma::Mat<eT>& output)
  {
    arma::Mat<eT> inputPadded = input;
    arma::Mat<eT> filterPadded = filter;

    if (padLastDim)
      inputPadded.resize(inputPadded.n_rows, inputPadded.n_cols + 1);

    // Pad filter and input to the output shape.
    filterPadded.resize(inputPadded.n_rows, inputPadded.n_cols);

    output = arma::real(ifft2(arma::fft2(inputPadded) % arma::fft2(
        filterPadded)));

    // Extract the region of interest. We don't need to handle the padLastDim in
    // a special way we just cut it out from the output matrix.
    output = output.submat(filter.n_rows - 1, filter.n_cols - 1,
        input.n_rows - 1, input.n_cols - 1);
  }

  /*
   * Perform a convolution through fft (full mode). This method only supports
   * input which is even on the last dimension. In case of an odd input with, a
   * user can manually pad the imput or specify the padLastDim parameter which
   * takes care of padding. The filter instead can have any size.
   *
   * @param input Input used to perform the convolution.
   * @param filter Filter used to perform the conolution.
   * @param output Output data that contains the results of the convolution.
   */
  template<typename eT, typename Border = BorderMode>
  static typename std::enable_if<
      std::is_same<Border, FullConvolution>::value, void>::type
  Convolution(const arma::Mat<eT>& input,
              const arma::Mat<eT>& filter,
              arma::Mat<eT>& output)
  {
    // In case of the full convolution outputRows and outputCols doesn't
    // represent the true output size when the padLastDim parameter is set,
    // instead it's the working size.
    const size_t outputRows = input.n_rows + 2 * (filter.n_rows - 1);
    size_t outputCols = input.n_cols + 2 * (filter.n_cols - 1);

    if (padLastDim)
        outputCols++;

    // Pad filter and input to the working output shape.
    arma::Mat<eT> inputPadded = arma::zeros<arma::Mat<eT> >(outputRows,
        outputCols);
    inputPadded.submat(filter.n_rows - 1, filter.n_cols - 1,
          filter.n_rows - 1 + input.n_rows - 1,
          filter.n_cols - 1 + input.n_cols - 1) = input;

    arma::Mat<eT> filterPadded = filter;
    filterPadded.resize(outputRows, outputCols);

    // Perform FFT and IFFT
    output = arma::real(ifft2(arma::fft2(inputPadded) % arma::fft2(
        filterPadded)));

    // Extract the region of interest. We don't need to handle the padLastDim
    // parameter in a special way we just cut it out from the output matrix.
    output = output.submat(filter.n_rows - 1, filter.n_cols - 1,
        2 * (filter.n_rows - 1) + input.n_rows - 1,
        2 * (filter.n_cols - 1) + input.n_cols - 1);
  }
};  // class FFTConvolution

}; // namespace ann
}; // namespace mlpack

#endif
