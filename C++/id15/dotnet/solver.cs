using System;
using System.Drawing;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;
using System.Data;

using System.Runtime.InteropServices;


namespace id15NET
{	
	/// <summary>
	/// Summary description for Form1.
	/// </summary>
	
	public class Form1 : System.Windows.Forms.Form
	{
		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.Container components = null;

		public Form1()
		{
			//
			// Required for Windows Form Designer support
			//
			InitializeComponent();

			//
			// TODO: Add any constructor code after InitializeComponent call
			//
		}

		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		protected override void Dispose( bool disposing )
		{
			if( disposing )
			{
				if (components != null) 
				{
					components.Dispose();
				}
			}
			base.Dispose( disposing );
		}

		#region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
			this.button1 = new System.Windows.Forms.Button();
			this.SuspendLayout();
			// 
			// button1
			// 
			this.button1.Location = new System.Drawing.Point(208, 80);
			this.button1.Name = "button1";
			this.button1.TabIndex = 0;
			this.button1.Text = "button1";
			this.button1.Click += new System.EventHandler(this.button1_Click);
			// 
			// Form1
			// 
			this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
			this.ClientSize = new System.Drawing.Size(292, 271);
			this.Controls.Add(this.button1);
			this.Name = "Form1";
			this.Text = "Form1";
			this.ResumeLayout(false);

		}
		#endregion

		/// <summary>
		/// The main entry point for the application.
		/// </summary>
		[STAThread]
		static void Main() 
		{
			Application.Run(new Form1());
		}

		private System.Windows.Forms.Button button1;

		const int MRight = 0;
		const int MUp = 1;
		const int MLeft = 2;
		const int MDown = 3;
		

		const int BOARD_MAXWIDTH      = 5;
		const int BOARD_MAXHEIGHT     = 5;
		const int BOARD_MAXCELLSCOUNT = BOARD_MAXWIDTH * BOARD_MAXHEIGHT;
		const int MOVES_NUMBER        = 4;
		const int MOVES_MAX           = 1024;
		const int DEPTH_MAX           = 63;

		int FColCount;
		int FRowCount;
		int FCellCount;

		int[,] FCellsRating;
		int[,] FVectSet;
		int[,,] FTurnSet;

		bool Solve(
			int AWidth,
			int AHeight,
			int[]ABoard,
			bool AAuto,
			int AThinkDepth, 
			out Int64 APositionsNum, 
			out ArrayList retpath,
			out int size
			//TFifteenCallback AFifteenCallback
			)
		{

			
			retpath = new ArrayList();
			int[] FCompletePath = new int[MOVES_MAX];
			int minsum, sum;
			int OnPlace;
			int i, j, k, PathSetPos;
			int NewTop;
			int iThinkDepth;
			int[] path = new int[255];
			int[] maxpath = new int[255];
			int[] vect = new int[255];

			FColCount = AWidth;
			FRowCount = AHeight;
			FCellCount = FColCount*FRowCount;

			FCellsRating = new int[FCellCount, FCellCount];
			FVectSet = new int[FCellCount, FCellCount];
			FTurnSet = new int[MOVES_NUMBER,FCellCount,MOVES_NUMBER + 1];

			int[] PathSet = new int[DEPTH_MAX];
			int[] Board = new int[FCellCount];
			int[] OldBoard = new int[FCellCount];

			int MaxThinkDepth = AThinkDepth - FColCount; 
			int Hole = -1;
			int PrevTop = -1;
			int maxDepth = 0;
			int TurnsNum = 0;
			Int64 PositionsNum = 0;
			bool Breaked = false;
			bool Result = false;

			for(i = 0; i < FCellCount; i++)
			{
				Board[i] = ABoard[i];
				if (Board[i] == FCellCount - 1)
					Hole = i;
			}

			// Проверка
			OnPlace = 0;
			while (OnPlace < FCellCount && Board[OnPlace] == OnPlace)
				OnPlace++;

			Result = (OnPlace == FCellCount - 1);

			if (!AAuto) 
			{
				InitArrays(0, FRowCount - 1);
				MaxThinkDepth += FColCount;
			}
			while (!Breaked && !Result) 
			{
				minsum = 32767;
				NewTop = OnPlace / FColCount;
				if (/*AAuto && */NewTop != PrevTop) 
				{
					if (FRowCount - 1 <= NewTop) 
						NewTop = FRowCount - 2;
					PrevTop = NewTop;
					InitArrays(NewTop, FRowCount - 1);
					MaxThinkDepth += FColCount;
				}

				for (iThinkDepth = 1; iThinkDepth <= MaxThinkDepth; iThinkDepth++) 
				{
					if(OnPlace / FColCount!= PrevTop && FRowCount - 1 > OnPlace / FColCount)
						break;
					if (Breaked) 
						break;
					//if (AFifteenCallback) 
					//	Breaked = AFifteenCallback(iThinkDepth, OnPlace, TurnsNum, PositionsNum);
					path[0] = Hole;
					for (k = 0; k < 4; k++) 
					{
						if (FTurnSet[k,Hole,0] == 0)
							continue;
						vect[0]    = k;
						PathSetPos = 1;
						//CopyMemory(OldBoard, Board, FCellCount * sizeof(Board[0]));
						Array.Copy(Board,0,OldBoard, 0, FCellCount);
						//for(int ii = 0; ii < FCellCount; ii++)
						//OldBoard[ii] = Board[ii];
						
						for (i = 0; i <= iThinkDepth; i++)
							PathSet[i] = 1;

						do 
						{
							PositionsNum++; // Счетчик перебранных позиций

							// Накат
							for (j = PathSetPos; j <= iThinkDepth; j++) 
							{
								path[j] = FTurnSet[vect[j - 1],path[j - 1],PathSet[j]];
								vect[j] = FVectSet[path[j - 1],path[j]];
								OldBoard[path[j - 1]] = OldBoard[path[j]];
							}
							OldBoard[path[iThinkDepth]] = FCellCount - 1;

							// Проверка
							i = 0;
						while (i < FCellCount && OldBoard[i] == i)
							i++;
							Result = (i == FCellCount);

							// Оценка
							if (i == OnPlace) 
							{
								if (iThinkDepth == maxDepth || OnPlace == 0) 
								{
									sum = 0;
									for (j = OnPlace; j < FCellCount; j++) 
									{
										if (OldBoard[j] == OnPlace)
											sum += 256 * FCellsRating[OldBoard[j],j];
										if (OldBoard[j] == OnPlace + 1 && OldBoard[j] != FCellCount - 1)
											sum += Math.Abs(OldBoard[j] - j) * FCellsRating[OldBoard[j],j];
									}
									if (sum != 0 && sum < minsum) 
									{
										minsum   = sum;
										maxDepth = iThinkDepth;
							
										//CopyMemory(&maxpath[0], &path[0], (iThinkDepth + 1) * sizeof(path[0]));

										Array.Copy(path, 0, maxpath, 0, iThinkDepth + 1);
										//for(int ii = 0; ii < iThinkDepth + 1; ii++)
										//	maxpath[ii] = path[ii];

									}
								}
							} 
							else 
							{
								if (i > OnPlace) 
								{
									OnPlace  = i;
									maxDepth = iThinkDepth;

									//CopyMemory(&maxpath[0], &path[0], (iThinkDepth + 1) * sizeof(path[0]));
									Array.Copy(path, 0, maxpath, 0, iThinkDepth + 1);
									//for(int ii = 0; ii < iThinkDepth + 1; ii++)
//										maxpath[ii] = path[ii];
									//if (AFifteenCallback) 
									//  Breaked = AFifteenCallback(iThinkDepth, OnPlace, TurnsNum + maxDepth, PositionsNum);
								}
							}

							// Следующий путь
							PathSetPos = iThinkDepth;
						while (PathSet[PathSetPos] == 
							FTurnSet[vect[PathSetPos - 1],path[PathSetPos - 1],0] 
							&& PathSetPos != 1)
						{
							PathSet[PathSetPos] = 1;
							PathSetPos--;
						}
							PathSet[PathSetPos]++;

							// Откат
							for (j = iThinkDepth; j >= PathSetPos; j--)
								OldBoard[path[j]] = OldBoard[path[j - 1]];
							OldBoard[path[PathSetPos - 1]] = FCellCount - 1;

						} while (!Result && PathSetPos != 1);
						if (Result) break;
					}
					if (Result) break;
				}
				// Сохранение текущего найденого пути
				if (!Result && OnPlace < FCellCount - FRowCount) 
				{
					if (maxDepth > 16) 
						maxDepth = 16;
				}
				//FCompletePath = (int*) my_realloc(FCompletePath, sizeof(int) * (TurnsNum + maxDepth));
				for (i = TurnsNum; i <= TurnsNum + maxDepth - 1; i++)
					FCompletePath[i] = maxpath[i - TurnsNum + 1];
				TurnsNum += maxDepth;
				if (TurnsNum > MOVES_MAX) break;
				for (i = 1; i <= maxDepth; i++)
					Board[maxpath[i - 1]] = Board[maxpath[i]];
				//if (maxDepth > 0)
				Board[maxpath[maxDepth]] = FCellCount - 1;
				OnPlace = 0;
				while (OnPlace < FCellCount && Board[OnPlace] == OnPlace) 
					OnPlace++;
				Hole = maxpath[maxDepth];
			};

			//RemoveUnnecessary(FCompletePath);
			//TurnsNum = FCompletePath.Length;
			//if (AFifteenCallback) 
			//AFifteenCallback(MaxThinkDepth, OnPlace, TurnsNum, PositionsNum);
			APositionsNum = PositionsNum;

			size = TurnsNum;
			//CopyMemory(retpath, FCompletePath, TurnsNum * sizeof(FCompletePath[0]));
			//retpath = FCompletePath;

			for(int ii = 0; ii < TurnsNum; ii++)
				retpath.Add(FCompletePath[ii]);
			


			return Result;
		}

		void RemoveUnnecessary(int[] chain)
		{
			uint i = 0;
			if (chain.Length > 0)
			{
				while (i <= chain.Length - 2)
				{
					while (chain[i] == chain[i + 2] && i <= chain.Length - 2)
					{
						for (uint j = i + 2; j <= chain.Length; j++)
							chain[j - 2] = chain[j];

						//chain = (int*) my_realloc(chain, sizeof(int) * (chain.Length - 1));
						if (i > 0) i--;
						if (i > 0) i--;
					}
					i++;
				}
			}
		}

		void InitArrays(int Top, int Bottom)
		{
			int i, j, k, x, y, m, OldM;
			int Turns, OldTurns;
			//  FVectSet = (int**) malloc(sizeof(int) * FCellCount * FCellCount);
			//  FTurnSet = (int***) malloc(sizeof(int) * (MOVES_NUMBER * FCellCount * (MOVES_NUMBER + 1)));

			for (i = 0; i < FCellCount; i++)
				for (j = 0; j < FCellCount; j++)
					FVectSet[i,j] = -1;

			for (i = 0; i < 4; i++)
				for (j = 0; j < FCellCount; j++)
					for (k = 0; k < 5; k++)
						FTurnSet[i,j,k] = 0;

			for (i = 0; i < FCellCount; i++) 
			{
				x = i % FColCount;
				y = i / FColCount;
				if (y > Top)           FVectSet[i,i - FColCount] = MUp;
				if (x > 0)             FVectSet[i,i - 1] = MLeft;
				if (y < FRowCount - 1) FVectSet[i,i + FColCount] = MDown;
				if (x < FColCount - 1) FVectSet[i,i + 1] = MRight;
			}

			for (i = 0; i < 4; i++) 
			{
				OldTurns = 0xf; //00001111b
				OldM     = -1;
				switch (i) 
				{
					case MRight:
						if (FRowCount != 1) OldM = MLeft;
						break;
					case MUp:   
						if (FColCount != 1) OldM = MDown;
						break;
					case MLeft: 
						if (FRowCount != 1) OldM = MRight;
						break;
					case MDown: 
						if (FColCount != 1) OldM = MUp;
						break;
				}

				if(OldM != -1)
					OldTurns &= ~(1<<OldM);

				for (j = 0; j < FCellCount; j++) 
				{
					x = j % FColCount;
					y = j / FColCount;
					if (y == Top && i == MDown) continue;
					if (x == 0 && i == MRight) continue;
					if (y == FRowCount - 1 && i == MUp) continue;
					if (x == FColCount - 1 && i == MLeft) continue;

					Turns = OldTurns;

					if (y == Top)
						Turns &= ~(1<<MUp);
					if (x == 0)
						Turns &= ~(1<<MLeft);
					if (y == FRowCount - 1)
						Turns &= ~(1<<MDown);
					if (x == FColCount - 1)
						Turns &= ~(1<<MRight);

					for (k = i + 3; k <= i + 6; k++) 
					{
						m = k % 4;
						if ((Turns & (1<<m)) > 0) 
						{
							FTurnSet[i,j,0]++;
							switch (m) 
							{
								case MRight: 
									m = j + 1;
									break;
								case MUp:    
									m = j - FColCount;
									break;
								case MLeft:  
									m = j - 1;
									break;
								case MDown:  
									m = j + FColCount;
									break;
							}
							FTurnSet[i,j,FTurnSet[i,j,0]] = m;
						}
					}
				}
			}

			for (i = 0; i < FCellCount; i++)
				for (j = 0; j < FCellCount; j++)
					FCellsRating[i,j] = Math.Abs(i/FColCount - j/FColCount) + Math.Abs(i%FColCount - j%FColCount);
		}

		public void testtest()
		{
			MessageBox.Show("asdfa");
		}


		private void button1_Click(object sender, System.EventArgs e)
		{			
			int[]ABoard = {15, 14,13,12,11,10,9,8,7,6,5,4,3,2,1,0};
						 
			Int64 APositionsNum;
			ArrayList retpath;
			int size;

			DateTime dt1 = DateTime.Now;
			Solve(4, 4, ABoard, true, 20, out APositionsNum, out retpath, out size);
			button1.Text = ((double)(DateTime.Now.Ticks - dt1.Ticks)/10000000).ToString();
		}
			
	}
}
