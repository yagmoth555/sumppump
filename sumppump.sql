SET SQL_MODE = "NO_AUTO_VALUE_ON_ZERO";
SET AUTOCOMMIT = 0;
START TRANSACTION;
SET time_zone = "+00:00";

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;


CREATE TABLE `comment` (
  `comment_id` int(11) NOT NULL,
  `user_id` int(11) NOT NULL,
  `edited` int(11) NOT NULL,
  `score` int(11) NOT NULL,
  `post_id` int(11) NOT NULL,
  `link` varchar(255) NOT NULL,
  `reply_to_user` int(11) NOT NULL,
  `site_id` int(11) NOT NULL,
  `name` varchar(255) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE `question` (
  `question_id` int(11) NOT NULL,
  `visit` date NOT NULL,
  `link` varchar(255) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE `site` (
  `site_id` int(11) NOT NULL,
  `site` varchar(255) NOT NULL,
  `visit` date NOT NULL,
  `link` varchar(255) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE `sumppump` (
  `quota_max` int(11) NOT NULL,
  `quota_remaining` int(11) NOT NULL,
  `visit` date NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE `user` (
  `user_id` int(11) NOT NULL,
  `reputation` int(11) NOT NULL,
  `user_type` int(11) NOT NULL,
  `accept_rate` int(11) NOT NULL,
  `profile_image` varchar(255) NOT NULL,
  `display_name` varchar(255) NOT NULL,
  `link` varchar(255) NOT NULL,
  `site_id` int(11) NOT NULL,
  `name` varchar(255) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;


ALTER TABLE `comment`
  ADD PRIMARY KEY (`comment_id`);

ALTER TABLE `question`
  ADD PRIMARY KEY (`question_id`);

ALTER TABLE `site`
  ADD PRIMARY KEY (`site_id`);

ALTER TABLE `user`
  ADD PRIMARY KEY (`user_id`);


ALTER TABLE `site`
  MODIFY `site_id` int(11) NOT NULL AUTO_INCREMENT;
COMMIT;

/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
