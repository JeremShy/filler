#include <filler.h>

void	calculate_all_distances(t_d *data)
{
	t_p	*tmp;

	tmp = data->points;
	while (tmp)
	{
		tmp->distance =	calculate_distance(data, tmp);
		tmp = tmp->next;
	}
}

int		calculate_distance_piece(t_d *data, int x, int y)
{
	t_p point;

	point.x = x;
	point.y = y;
	return (calculate_distance(data, &point));
}

int		surrounded_by_points(t_d *data, int x, int y)
{
	int flag;

	flag = 0;
	if (x - 1 < 0 || data->board[x - 1][y] == data->us_max || data->board[x - 1][y] == data->other_max)
		flag++;
	if (y - 1 < 0 || data->board[x][y - 1 ] == data->us_max  || data->board[x][y - 1] == data->other_max)
		flag++;
	if (x + 1 >= data->size_x || data->board[x + 1][y] == data->us_max  || data->board[x + 1][y] == data->other_max)
		flag++;
	if (y + 1 >= data->size_y || data->board[x][y + 1] == data->us_max  || data->board[x][y + 1] == data->other_max)
		flag++;
	return (!(flag == 4));
}

int	is_possible_and_distance(t_d *data, int x, int y)
{
	int i;
	int j;
	int flag;
	int min;

	i = 0;
	flag = 0;
	min = -1;

	while (flag >= 0 && i < data->piece_x)
	{
		j = 0;
		if (data->piece[i][j] == '*' &&
			(y + j >= data->size_y || x + i >= data->size_x || y + j < 0 || x + i < 0))
		{
			flag = -1;
		}
		while (flag >= 0 && j < data->piece_y)
		{
			if (data->piece[i][j] == '*' &&
				(y + j >= data->size_y || x + i >= data->size_x || y + j < 0 || x + i < 0))
			{
				flag = -1;
				break;
			}
			if (data->piece[i][j] == '*')
				min = min != -1 && min < calculate_distance_piece(data, x + i, y + j) ? min : calculate_distance_piece(data, x + i, y + j);

			if (x + i >= 0 && y + j >= 0 && x + i < data->size_x && y + j < data->size_y)
			{
				if (data->board[x + i][y + j] == data->us_max && data->piece[i][j] == '*')
				 {
					 if (flag == 1)
					 	flag = -1;
					else
						flag = 1;
				 }
				 else
				if ((data->board[x + i][y + j] == data->other_min ||
					data->board[x + i][y + j] == data->other_max) && data->piece[i][j] == '*')
					flag = -1;
			}
			j++;
		}
		i++;
	}
	return (flag == 1 ? min : 0);
}

void	add_distance(t_p *ret, int distance)
{
	while (ret->next)
		ret = ret->next;
	ret->distance = distance;
}

t_p	*create_pos_list(t_d *data)
{
	t_p	*points;
	t_p	*ret;
	int	i;
	int	j;
	int distance;

	ret = NULL;
	points = data->points;
	// dprintf(data->debug_fd, "\n\n\n\nnew testing piece\n");
	while (points)
	{
		// dprintf(data->debug_fd, "testing point at %d-%d\n", points->x, points->y);
		i = 0;
		while (i < data->piece_x)
		{
			j = 0;
			while(j < data->piece_y)
			{
				if ((distance = is_possible_and_distance(data, points->x - i, points->y - j)))
				{
					// dprintf(data->debug_fd, "Possible for %d - %d\n", i, j);
					create_point(&ret, points->x - i, points->y - j);
					if (!ret)
					{
						// dprintf(data->debug_fd, "malloc error\n");
						return (NULL);
					}
					add_distance(ret, distance);
				}
				else
				{
					// dprintf(data->debug_fd, "Not possible for %d - %d\n", i, j);
				}
				j++;
			}
			i++;
		}
		if (ret)
		{
			ret = insert_sort(ret, func);
			return (ret);
		}
		points = points->next;
	}
	return (NULL);
}

void	purge_points(t_d *data)
{
	t_p	*tmp;
	t_p	*old;

	tmp = data->points;
	old = NULL;
	while (tmp)
	{
		if (surrounded_by_points(data, tmp->x, tmp->y))
		{
			dprintf(data->debug_fd, "Surrounded  x = %d, y = %d.\n", tmp->x, tmp->y);
			if (old)
				old->next = tmp->next;
			else
				data->points = tmp->next;
			old = tmp->next;
			free(tmp);
			tmp = old;
		}
		else
		{
			dprintf(data->debug_fd, "Not surrounded x = %d, y = %d.\n", tmp->x, tmp->y);
		}
		old = tmp;
		if (tmp)
			tmp = tmp->next;
	}
}

void	add_piece_to_board(t_d *data, t_p *coords)
{
	int	i;
	int	j;

	i = 0;
	while (i < data->piece_x)
	{
		j = 0;
		while (j < data->piece_y)
		{
			if (data->piece[i][j] == '*')
				create_point(&(data->points), coords->x + i, coords->y + j);
			j++;
		}
		i++;
	}
	// purge_points(data);
}

int	put_piece(t_d *data)
{
	int i;
	char *tmp;
	t_p		*positions;

	i = 0;
	if (data->first_round)
	{
		while (data->board[i])
		{
			if ((tmp = ft_strchr(data->board[i], data->us_max)))
				if (!create_point(&(data->points), i, (int)(tmp - data->board[i])))
				{
					// dprintf(data->debug_fd, "error 0 !\n");
					return (0);
				}
			i++;
		}
	}
	calculate_all_distances(data);
	data->points = insert_sort(data->points, func);
	positions = create_pos_list(data);
	if (!positions)
	{
		// dprintf(data->debug_fd, "error 1\n");
		return (0);
	}
	// dprintf(data->debug_fd, "%d-%d\n", positions->x, positions->y);
	add_piece_to_board(data, positions);
	ft_printf("%d %d\n", positions->x, positions->y);
	// dellist(position);
	return (1);
}
