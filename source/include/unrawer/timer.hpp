/*
 * UnRAWer - camera raw bnatch processor on top of OpenImageIO
 * Copyright (c) 2022 Erium Vladlen.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once
#ifndef _UNRAWER_TIMER_HPP
#define _UNRAWER_TIMER_HPP

#include <chrono>
#include <string>

namespace unrw
{
    class Timer {
    public:
        Timer();
        ~Timer();

        template <typename T>
        T now() const;

        const std::string nowText(int w = 0, int p = 6) const;

        friend std::ostream& operator<<(std::ostream& os, const Timer& timer);

    private:
        std::chrono::high_resolution_clock::time_point start_;
    };

    template float Timer::now<float>() const;
    template double Timer::now<double>() const;

    std::ostream& operator<<(std::ostream& os, const Timer& timer);
}
+
#endif // !_UNRAWER_TIMER_HPP
