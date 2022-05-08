-- ================================================
-- Template generated from Template Explorer using:
-- Create Procedure (New Menu).SQL
--
-- Use the Specify Values for Template Parameters 
-- command (Ctrl-Shift-M) to fill in the parameter 
-- values below.
--
-- This block of comments will not be included in
-- the definition of the procedure.
-- ================================================
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
-- =============================================
-- Author:		<Author,,Name>
-- Create date: <Create Date,,>
-- Description:	<Description,,>
-- =============================================
CREATE PROCEDURE [dbo].[saveUserInformation]
	-- Add the parameters for the stored procedure here
	@userID NCHAR(10),
	@level INT,
	@positionX INT,
	@positionY INT,
	@experiencePoint INT,
	@healthPoint INT
AS
BEGIN
	-- SET NOCOUNT ON added to prevent extra result sets from
	-- interfering with SELECT statements.
	SET NOCOUNT ON;

    -- Insert statements for procedure here
	UPDATE [dbo].[userData]
	SET [level] = @level, positionX = @positionX, positionY = @positionY, experiencePoint = @experiencePoint, healthPoint = @healthPoint
	WHERE userID = @userID
END
GO
