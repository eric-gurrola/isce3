#include <isce/core/NewOrbit.h>
#include <isce/except/Error.h>
#include <gtest/gtest.h>

struct OrbitTest : public testing::Test {

    std::vector<isce::core::StateVector> statevecs;

    isce::core::DateTime refepoch;
    isce::core::TimeDelta spacing;
    int size;

    void SetUp() override
    {
        statevecs.resize(2);

        statevecs[0].datetime = "2016-04-08T09:13:13.000000";
        statevecs[0].position = {-3752316.976337, 4925051.878499, 3417259.473609};
        statevecs[0].velocity = {3505.330104, -1842.136554, 6482.122476};

        statevecs[1].datetime = "2016-04-08T09:13:23.000000";
        statevecs[1].position = {-3717067.52658, 4906329.056304, 3481886.455117};
        statevecs[1].velocity = {3544.479224, -1902.402281, 6443.152265};

        refepoch = statevecs[0].datetime;
        spacing = statevecs[1].datetime - statevecs[0].datetime;
        size = statevecs.size();
    }
};

TEST_F(OrbitTest, FromStateVectors)
{
    typedef isce::core::NewOrbit Orbit;

    Orbit orbit = Orbit::from_statevectors(statevecs);

    EXPECT_EQ( orbit.position()[0], statevecs[0].position );
    EXPECT_EQ( orbit.velocity()[0], statevecs[0].velocity );

    EXPECT_EQ( orbit.position()[1], statevecs[1].position );
    EXPECT_EQ( orbit.velocity()[1], statevecs[1].velocity );
}

TEST_F(OrbitTest, BasicConstructor)
{
    isce::core::NewOrbit orbit (refepoch, spacing, size);

    EXPECT_EQ( orbit.refepoch(), refepoch );
    EXPECT_EQ( orbit.spacing(), spacing );
    EXPECT_EQ( orbit.size(), size );
}

TEST_F(OrbitTest, Accessor)
{
    isce::core::NewOrbit orbit (refepoch, spacing, size);

    orbit[0] = statevecs[0];
    orbit[1] = statevecs[1];

    EXPECT_EQ( orbit[0], statevecs[0] );
    EXPECT_EQ( orbit[1], statevecs[1] );
}

TEST_F(OrbitTest, ConstAccessor)
{
    typedef isce::core::NewOrbit Orbit;

    const Orbit orbit = Orbit::from_statevectors(statevecs);

    EXPECT_EQ( orbit[0], statevecs[0] );
    EXPECT_EQ( orbit[1], statevecs[1] );
}

TEST_F(OrbitTest, AccessorDateTimeMismatch)
{
    typedef isce::core::NewOrbit Orbit;

    Orbit orbit = Orbit::from_statevectors(statevecs);

    isce::core::StateVector statevec = statevecs[0];
    statevec.datetime += 1.;

    // datetime of statevector must be = refepoch() + spacing() * idx
    EXPECT_THROW( orbit[0] = statevec, isce::except::InvalidArgument );
}

TEST_F(OrbitTest, PushBack)
{
    isce::core::NewOrbit orbit (refepoch, spacing);

    orbit.push_back(statevecs[0]);
    orbit.push_back(statevecs[1]);

    EXPECT_EQ( orbit.time()[0], 0. );
    EXPECT_EQ( orbit.position()[0], statevecs[0].position );
    EXPECT_EQ( orbit.velocity()[0], statevecs[0].velocity );

    EXPECT_EQ( orbit.time()[1], spacing.getTotalSeconds() );
    EXPECT_EQ( orbit.position()[1], statevecs[1].position );
    EXPECT_EQ( orbit.velocity()[1], statevecs[1].velocity );
}

TEST_F(OrbitTest, PushBackDateTimeMismatch)
{
    isce::core::NewOrbit orbit (refepoch, spacing);

    isce::core::StateVector statevec = statevecs[0];
    statevec.datetime += 1.;

    // datetime of next statevector must be = refepoch() + spacing() * size()
    EXPECT_THROW( orbit.push_back(statevec), isce::except::InvalidArgument );
}

TEST_F(OrbitTest, Resize)
{
    isce::core::NewOrbit orbit (refepoch, spacing, size);

    int new_size = 5;
    orbit.resize(new_size);

    EXPECT_EQ( orbit.size(), new_size );
}

TEST_F(OrbitTest, Empty)
{
    {
        isce::core::NewOrbit orbit (refepoch, spacing, 0);
        EXPECT_TRUE( orbit.empty() );
    }

    {
        isce::core::NewOrbit orbit (refepoch, spacing, size);
        EXPECT_FALSE( orbit.empty() );
    }
}

TEST_F(OrbitTest, ToStateVectors)
{
    isce::core::NewOrbit orbit (refepoch, spacing);

    orbit.push_back(statevecs[0]);
    orbit.push_back(statevecs[1]);

    std::vector<isce::core::StateVector> orbit_statevecs = orbit.to_statevectors();

    EXPECT_EQ( orbit_statevecs, statevecs );
}

TEST_F(OrbitTest, Comparison)
{
    typedef isce::core::NewOrbit Orbit;

    Orbit orbit1 = Orbit::from_statevectors(statevecs);
    Orbit orbit2 = Orbit::from_statevectors(statevecs);
    Orbit orbit3;

    EXPECT_TRUE( orbit1 == orbit2 );
    EXPECT_TRUE( orbit1 != orbit3 );
}

int main(int argc, char * argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

