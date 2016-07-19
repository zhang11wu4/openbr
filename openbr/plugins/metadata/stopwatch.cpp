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

#include <opencv2/opencv.hpp>
using namespace cv;

#include <openbr/plugins/openbr_internal.h>

#include <iostream>
using namespace std;

namespace br
{

/*!
 * \ingroup transforms
 * \brief Gives time elapsed over a specified Transform as a function of both images (or frames) and pixels.
 * \author Jordan Cheney \cite JordanCheney
 * \author Josh Klontz \cite jklontz
 */
class StopWatchTransform : public MetaTransform
{
    Q_OBJECT
    Q_PROPERTY(QString description READ get_description WRITE set_description RESET reset_description)
    BR_PROPERTY(QString, description, "Identity")

    br::Transform *transform;
    mutable QMutex mutex;
    mutable long milliseconds;
    mutable long images;
    mutable long pixels;

public:
    StopWatchTransform()
    {
        reset();
    }

private:
    void reset()
    {
        milliseconds = 0;
        images = 0;
        pixels = 0;
    }

    void init()
    {
        transform = Transform::make(description);
    }

    void train(const QList<TemplateList> &data)
    {
        QTime watch;
        watch.start();

        transform->train(data);

        milliseconds += watch.elapsed();
        images += data.size();

        // compute the total number of pixels we processed
        foreach(const TemplateList &list, data) {
          const Mat& m = list.first().m();
          pixels += m.rows*m.cols;
        }
    }

    void project(const Template &src, Template &dst) const
    {
        QTime watch;
        watch.start();
        transform->project(src, dst);

        QMutexLocker locker(&mutex);
        milliseconds += watch.elapsed();
        images++;
        foreach (const cv::Mat &m, src)
            pixels += (m.rows * m.cols);
    }

    void finalize(TemplateList &)
    {
        qDebug("\nProfile for \"%s\"\n"
               "\tSeconds: %g\n"
               "\tTemplates/s: %g\n"
               "\tPixels/s: %g\n",
               qPrintable(description),
               milliseconds / 1000.0,
               images * 1000.0 / milliseconds,
               pixels * 1000.0 / milliseconds);
        reset();
    }

    void store(QDataStream &stream) const
    {
        transform->store(stream);

        qDebug("\nTraining Profile for \"%s\"\n"
               "\tSeconds: %g\n"
               "\tTemplates/s: %g\n"
               "\tPixels/s: %g\n",
               qPrintable(description),
               milliseconds / 1000.0,
               images * 1000.0 / milliseconds,
               pixels * 1000.0 / milliseconds);
    }

    void load(QDataStream &stream)
    {
        transform->load(stream);
    }
};

BR_REGISTER(Transform, StopWatchTransform)

} // namespace br

#include "metadata/stopwatch.moc"
