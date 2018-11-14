function v = HELICS_DATA_TYPE_CHAR()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1398230932);
  end
  v = vInitialized;
end
