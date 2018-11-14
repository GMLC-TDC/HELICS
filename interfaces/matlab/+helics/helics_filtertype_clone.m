function v = helics_filtertype_clone()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1398230886);
  end
  v = vInitialized;
end
