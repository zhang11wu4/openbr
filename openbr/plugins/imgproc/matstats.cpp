/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright 2012 The MITRE Corporation                                      *
 *                                                                           *
 * Licensed under the Apache License, Version 2.0 (the "License");           *
 * you may not use this file except in compliance with the License.          *
 * You may obtain a copy of the License at                                   *
 *                                                                           *
 *     http://www.apache.org/licenses/LICENSE-2.0                            *
 *                                                                           *
 * Unless required by applicable law or agreed to in writing, software       *
 * distributed under the License is distributed on an "AS IS" BASIS,         *
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  *
 * See the License for the specific language governing permissions and       *
 * limitations under the License.                                            *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <openbr/plugins/openbr_internal.h>

using namespace cv;

namespace br
{

/*!
 * \ingroup transforms
 * \brief Statistics
 * \author Josh Klontz \cite jklontz
 */
class MatStatsTransform : public UntrainableTransform
{
    Q_OBJECT
    Q_ENUMS(Statistic)
    Q_PROPERTY(Statistic statistic READ get_statistic WRITE set_statistic RESET reset_statistic STORED false)

public:
    /*!
     * \brief Available statistics
     */
    enum Statistic { Min, Max, Mean, StdDev };

private:
    BR_PROPERTY(Statistic, statistic, Mean)

    void project(const Template &src, Template &dst) const
    {
        if (src.m().channels() != 1)
            qFatal("Expected 1 channel matrix.");
        Mat m(1, 1, CV_32FC1);
        if ((statistic == Min) || (statistic == Max)) {
            double min, max;
            minMaxLoc(src, &min, &max);
            m.at<float>(0,0) = (statistic == Min ? min : max);
        } else {
            Scalar mean, stddev;
            meanStdDev(src, mean, stddev);
            m.at<float>(0,0) = (statistic == Mean ? mean[0] : stddev[0]);
        }
        dst = m;
    }
};

BR_REGISTER(Transform, MatStatsTransform)

} // namespace br

#include "imgproc/matstats.moc"
