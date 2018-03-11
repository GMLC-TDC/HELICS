function v = iterating()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 17);
  end
  v = vInitialized;
end
