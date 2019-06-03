#include <isce/core/InterpOrbit.h>
#include <isce/core/NewOrbit.h>
#include <gtest/gtest.h>

using namespace isce::core;

// make state vector from linear platform trajectory
StateVector make_linear_statevec(
        const DateTime & refepoch,
        const TimeDelta & dt,
        const Vec3 & initial_pos,
        const Vec3 & velocity)
{
    double _dt = dt.getTotalSeconds();
    return {refepoch + _dt, initial_pos + _dt * velocity, velocity};
}

// make orbit with linear platform trajectory
NewOrbit make_linear_orbit(
        const DateTime & refepoch,
        const TimeDelta & spacing,
        const Vec3 & initial_pos,
        const Vec3 & velocity,
        int size)
{
    NewOrbit orbit (refepoch, spacing, size);

    for (int i = 0; i < size; ++i) {
        TimeDelta dt = spacing * i;
        orbit[i] = make_linear_statevec(refepoch, dt, initial_pos, velocity);
    }

    return orbit;
}

struct InterpOrbitTest : public testing::Test {
    NewOrbit orbit;
    std::vector<double> test_times;
    std::vector<StateVector> expected;

    void SetUp() override
    {
        // 11 state vectors spaced 10s apart
        DateTime refepoch (2000, 1, 1);
        TimeDelta spacing = 10.;
        Vec3 initial_pos {0., 0., 0.};
        Vec3 velocity {4000., -1000., 4500.};
        int size = 11;

        orbit = make_linear_orbit(refepoch, spacing, initial_pos, velocity, size);

        test_times = {23.3, 36.7, 54.5, 89.3};

        for (size_t i = 0; i < test_times.size(); ++i) {
            TimeDelta dt = test_times[i];
            StateVector sv = make_linear_statevec(refepoch, dt, initial_pos, velocity);
            expected.push_back(sv);
        }
    }
};

bool isclose(const Vec3 & lhs, const Vec3 & rhs)
{
    double errtol = 1e-6;
    return std::abs(lhs[0] - rhs[0]) < errtol &&
           std::abs(lhs[1] - rhs[1]) < errtol &&
           std::abs(lhs[2] - rhs[2]) < errtol;
}

std::ostream & operator<<(std::ostream & os, const Vec3 & v)
{
    return os << std::endl << "{ " << v[0] << ", " << v[1] << ", " << v[2] << " }";
}

TEST_F(InterpOrbitTest, HermiteInterpolate)
{
    for (size_t i = 0; i < test_times.size(); ++i) {
        Vec3 position, velocity;
        hermite_interpolate(orbit, test_times[i], &position, &velocity);

        EXPECT_PRED2( isclose, position, expected[i].position );
        EXPECT_PRED2( isclose, velocity, expected[i].velocity );
    }
}

TEST_F(InterpOrbitTest, LegendreInterpolate)
{
    for (size_t i = 0; i < test_times.size(); ++i) {
        Vec3 position, velocity;
        legendre_interpolate(orbit, test_times[i], &position, &velocity);

        EXPECT_PRED2( isclose, position, expected[i].position );
        EXPECT_PRED2( isclose, velocity, expected[i].velocity );
    }
}

TEST_F(InterpOrbitTest, SCHInterpolate)
{
    for (size_t i = 0; i < test_times.size(); ++i) {
        Vec3 position, velocity;
        sch_interpolate(orbit, test_times[i], &position, &velocity);

        EXPECT_PRED2( isclose, position, expected[i].position );
        EXPECT_PRED2( isclose, velocity, expected[i].velocity );
    }
}

int main(int argc, char * argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

