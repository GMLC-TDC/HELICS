function v = helics_flag_observer()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1398230887);
  end
  v = vInitialized;
end
